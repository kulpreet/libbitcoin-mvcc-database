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

template <typename T>
store<T>::store(const block_store_ptr store)
    : block_store_(store)
{
  if (block_store_ != nullptr) {
    raw_block* new_block = get_new_block();
    tuple_size_ = sizeof(T);
    num_slots_ = BLOCK_SIZE / tuple_size_;
    // insert block
    blocks_.push_back(new_block);
  }
  insertion_head_ = blocks_.begin();
}

template <typename T>
raw_block* store<T>::get_new_block()
{
    raw_block* new_block = block_store_->get();
    initialize_raw_block(new_block);
    return new_block;
}

template <typename T>
void store<T>::initialize_raw_block(raw_block* block)
{
    block->insert_head_ = 0;
    get_slot_bitmap(block)->unsafe_clear(num_slots_);
}

template <typename T>
raw_concurrent_bitmap*
store<T>::get_slot_bitmap(raw_block* block)
{
    return reinterpret_cast<raw_concurrent_bitmap *>(
        util::aligned_ptr(sizeof(uint64_t), block->content_));
}

} // storage
} // database
} // libbitcoin

#endif
