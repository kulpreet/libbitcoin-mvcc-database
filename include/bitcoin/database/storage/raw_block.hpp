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

#ifndef LIBBITCOIN_MVCC_RAW_BLOCK_HPP
#define LIBBITCOIN_MVCC_RAW_BLOCK_HPP

#include <atomic>
#include <bitcoin/system.hpp>

namespace libbitcoin {
namespace database {
namespace storage {

const auto BLOCK_SIZE = 1 << 20;

// A lot of this comes from cmu db's TBD terrier project
// https://github.com/cmu-db/terrier

///////////////////////////////////////////////////////////////////////////////
// Layout
// insert_head_ 4 bytes
// slot bitmap  8 bytes
// finally the rest is for the tuple slots 1MB - 12 bytes
///////////////////////////////////////////////////////////////////////////////


class alignas(BLOCK_SIZE) raw_block
{
public:
  /**
   * The insert head tells us where the next insertion will take
   * place.
   *
   * This counter is never decreased as slot recycling does not happen
   * on the fly with insertions. A background compaction process scans
   * through blocks and free up slots.

   * Since the block size is less then (1<<20) the uppper 12 bits of
   * insert_head_ is free. We use the first bit (1<<31) to indicate if
   * the block is insertable.  If the first bit is 0, the block is
   * insertable, otherwise one txn is inserting to this block
   */
  std::atomic<uint32_t> insert_head_;

  /**
   * Contents of the raw block, an array of bytes.
   * unint32_t reserved for insert_head_
   */
  uint8_t content_[BLOCK_SIZE - sizeof(uint32_t)];

  /**
   * Get the offset of this block. Because the first bit insert_head_ is used to indicate the status
   * of the block, we need to clear the status bit to get the real offset
   * @return the offset which tells us where the next insertion should take place
   */
  uint32_t get_insert_head() {
      return MAX_INT32 & insert_head_.load();
  }

  /**
   * Compare and swap block status to busy. Expect that block to be idle, if yes, set the block status to be busy
   * @param block the block to compare and set
   * @return true if the set operation succeeded, false if the block is already busy
   */
  bool set_busy_status()
  {
      uint32_t old_val = clear_bit(insert_head_.load());
      return insert_head_.compare_exchange_strong(old_val, set_bit(old_val));
  }

  /**
   * Compare and swap block status to idle. Expect that block to be busy, if yes, set the block status to be idle
   * @param block the block to compare and set
   * @return true if the set operation succeeded, false if the block is already idle
   */
  bool clear_busy_status() {
      uint32_t val = insert_head_.load();
      if (val != set_bit(val))
          return false;

      return insert_head_.compare_exchange_strong(val, clear_bit(val));
  }

  /**
   * Set the first bit of given val to 0, helper function used by ClearBlockBusyStatus
   * @param val the value to be set
   * @return the changed value with first bit set to 0
   */
  static uint32_t clear_bit(uint32_t val)
  {
      return val & MAX_INT32;
  }

  /**
   * Set the first bit of given val to 1, helper function used by SetBlockBusyStatus
   * @param val val the value to be set
   * @return the changed value with first bit set to 1
   */
  static uint32_t set_bit(uint32_t val)
  {
      return val | MIN_INT32;
  }
};

/**
 * Allocator for allocating raw blocks
 */
class block_allocator {
public:
    /**
     * Allocates a new object by calling its constructor.
     * @return a pointer to the allocated object.
     */
    raw_block* allocate()
    {
        return new raw_block();
    }

    /**
     * reuse a reused chunk of memory to be handed out again
     * @param reused memory location, possibly filled with junk bytes
     */
    void reuse(raw_block* const reused)
    {
        /* no operation required */
    }

    /**
     * deallocate the object by calling its destructor.
     * @param ptr a pointer to the object to be deleted.
     */
    void deallocate(raw_block *const ptr)
    {
        delete ptr;
    }
};

} // namespace storage
} // namespace database
} // namespace libbitcoin

#endif
