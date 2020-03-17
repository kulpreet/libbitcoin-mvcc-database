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

#ifndef LIBBITCOIN_MVCC_STORAGE_HPP
#define LIBBITCOIN_MVCC_STORAGE_HPP

#include <atomic>
#include <bitcoin/system.hpp>

#include <bitcoin/database/storage/raw_block.hpp>
#include <bitcoin/database/storage/object_pool.hpp>
#include <bitcoin/database/storage/slot.hpp>
#include <bitcoin/database/storage/slot_iterator.hpp>
#include <bitcoin/database/container/concurrent_bitmap.hpp>

namespace libbitcoin {
namespace database {
namespace storage {

using namespace container;

template <typename T>
class store
{
public:
    /**
     * Constructs a new storage for the give type T, using the given
     * block_stre as the source of its storage blocks.
     *
     * @param store the block store to use.
     */
    store(const block_store_ptr store);

    /**
     * Destructs store, frees all its blocks and any potential varlen
     * entries.
     */
    ~store();

    // Initialize a raw block
    void initialize_raw_block(raw_block*);

    // Read the slot bitmap from the raw block contents
    raw_concurrent_bitmap* get_slot_bitmap(raw_block*);

private:

    raw_block* get_new_block();

    block_store_ptr block_store_;
    std::list<raw_block*> blocks_;
    std::list<raw_block*>::iterator insertion_head_;

    uint32_t tuple_size_;
    uint32_t num_slots_;
};

} // namespace storage
} // namespace database
} // namespace libbitcoin

#include <bitcoin/database/impl/storage.ipp>

#endif
