/**
 * Copyright (c) 2011-2019 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBBITCOIN_MVCC_DATABASE_MVCC_STORAGE_IPP
#define LIBBITCOIN_MVCC_DATABASE_MVCC_STORAGE_IPP

#include <bitcoin/database/storage/storage.hpp>
#include <bitcoin/database/storage/util.hpp>

namespace libbitcoin {
namespace database {
namespace storage {

template <typename record>
store<record>::store(const block_pool_ptr pool)
    : block_pool_(pool)
{
    blocks_latch_ = std::make_shared<spinlatch>();
    if (block_pool_ != nullptr)
    {
        raw_block* new_block = get_new_block();
        record_size_ = sizeof(record);
        num_slots_in_block_ = BLOCK_SIZE / record_size_;
        // insert block
        blocks_.push_back(new_block);
    }
    insertion_head_ = blocks_.begin();
}

template <typename record>
store<record>::~store()
{
    scopedspinlatch guard(blocks_latch_);
    for (raw_block *block : blocks_)
        block_pool_->release(block);
}

template <typename record>
raw_block* store<record>::get_new_block()
{
    raw_block* new_block = block_pool_->get();
    initialize_raw_block(new_block);
    return new_block;
}

template <typename record>
void store<record>::initialize_raw_block(raw_block* block)
{
    block->insert_head_ = 0;
    get_slot_bitmap(block)->unsafe_clear(num_slots_in_block_);
}

template <typename record>
raw_concurrent_bitmap*
store<record>::get_slot_bitmap(raw_block* block)
{
    return reinterpret_cast<raw_concurrent_bitmap *>(
        util::aligned_ptr(sizeof(uint64_t), block->content_));
}

// Insertion header points to the first block that has free tuple
// slots. Once a txn arrives, it will start from the insertion header
// to find the first idle (no other txn is trying to get tuple slots
// in that block) and non-full block.
//
// If no such block is found, the txn will create a new block.  Before
// the txn writes to the block, it will set block status to busy.
//
// The first bit of block insert_head_ is used to indicate if the
// block is busy. If the first bit is 1, it indicates one txn is
// writing to the block.
template <typename record>
slot store<record>::insert(transaction_context& context,
    const record &to_insert)
{
  slot result;
  auto block = insertion_head_;

  while (true)
  {
      std::cerr << "block: " << *block << std::endl;
      std::cerr << "blocks begin: " << *(blocks_.begin()) << std::endl;
      std::cerr << "blocks end: " << *(blocks_.end()) << std::endl;
      if (block == blocks_.end())
      {
          raw_block *new_block = get_new_block();
          auto busy = new_block->set_busy_status();
          BITCOIN_ASSERT_MSG(busy, "Status of new block should not be busy");

          // No need to flip the busy status bit
          allocate_in(new_block, &result);

          // take latch
          scopedspinlatch guard(blocks_latch_);

          // insert block
          blocks_.push_back(new_block);
          block = --blocks_.end();
          break;
      }

      auto set_success = (*block)->set_busy_status();
      if (set_success)
      {
          // No one is inserting into this block
          if (allocate_in(*block, &result)) {
              // The block is not full, succeed
              break;
          }
          // Fail to insert into the block, flip back the status bit
          (*block)->clear_busy_status();
          // if the full block is the insertion_header, move the
          // insertion_header Next insert txn will search from the new
          // insertion_header
          check_move_head(block);
      }
      // The block is full or the block is being inserted by other
      // txn, try next block
      ++block;
  }

  (*block)->clear_busy_status();
  std::cerr << "calling insert into " << std::endl;
  insert_into(context, to_insert, result);
  std::cerr << "back from return into" << std::endl;

  return result;
}

template <typename record>
bool store<record>::allocate_in(raw_block* block, slot* use_slot)
{
  raw_concurrent_bitmap *bitmap =  get_slot_bitmap(block);
  const uint32_t start = block->get_insert_head();

  // We are not allowed to insert into this block any more
  if (start == num_slots_in_block_) return false;

  uint32_t pos = start;
  // We do not support concurrent insertion to the same block.
  // Assumption: Different threads cannot insert into the same block at
  // the same time.
  // If the block is not full, the function should always succeed, i.e.
  // flip should always return true.
  bool flip_res = bitmap->flip(pos, false);
  BITCOIN_ASSERT_MSG(flip_res, "flip should always succeed");
  *use_slot = slot(block, pos);
  block->insert_head_++;
  return true;
}

// We don't check if the block is full or not, we just move forward
template <typename record>
typename record::tuple_ptr
store<record>::read(const slot& from, const transaction_context& context,
    typename record::reader read_with) const
{
    std::cerr << "in store::read..." << std::endl;
    // Get mvcc record from memory pointed to by slot
    auto bytes = from.get_bytes();
    std::cerr << "in store::read... got bytes " << std::endl;
    auto ptr = reinterpret_cast<record*>(bytes);
    std::cerr << "in store::read... got ptr " << std::endl;

    // get the record obtained
    auto result = ptr->read_record(context, read_with);
    std::cerr << "in store::read... got result from read_record " << std::endl;

    // return the obtained record
    return result;
}

// We don't check if the block is full or not, we just move forward
template <typename record>
void store<record>::check_move_head(std::list<raw_block *>::iterator block_iter)
{
    scopedspinlatch guard_head(insert_head_latch_);
  if (block_iter == insertion_head_)
  {
      // move the insertion head to point to the next block
    insertion_head_++;
  }

  // If there are no more free blocks, create a new empty block and
  // point the insertion_head to it
  if (insertion_head_ == blocks_.end())
  {
      raw_block *new_block = get_new_block();
    // take latch
    scopedspinlatch guard_block(blocks_latch_);
    // insert block
    blocks_.push_back(new_block);
    // set insertion header to --end()
    insertion_head_ = --blocks_.end();
  }
}

template <typename record>
void store<record>::insert_into(transaction_context& context,
    const record& to_insert, slot use_slot)
{
    // type case slot into record, so we can use latch/commit methods.
    auto location = use_slot.get_bytes();
    auto destination = reinterpret_cast<record*>(location);
    std::cerr << "record as destination: " << destination << std::endl;

    // get latch on record
    destination->get_latch_for_write(context);

    // write from to_insert to destination
    to_insert.write_to(destination, context);

    std::cerr << "return from insert into" << std::endl;
}

} // storage
} // database
} // libbitcoin

#endif
