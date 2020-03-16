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

namespace libbitcoin {
namespace database {
namespace storage {

template <typename T>
store<T>::store(const block_store_ptr store)
    : block_store_(store)
{
  if (block_store_ != nullptr) {
    raw_block* new_block = get_new_block();
    // insert block
    blocks_.push_back(new_block);
  }
  insertion_head_ = blocks_.begin();
}

template <typename T>
raw_block* store<T>::get_new_block()
{
    raw_block* new_block = block_store_->get();
    // accessor_.InitializeRawBlock(this, new_block, layout_version_);
    // data_table_counter_.IncrementNumNewBlock(1);
    return new_block;
}

} // storage
} // database
} // libbitcoin

#endif
