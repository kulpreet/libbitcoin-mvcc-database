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

#ifndef LIBBITCOIN_MVCC_MEMORY_HPP
#define LIBBITCOIN_MVCC_MEMORY_HPP

#include <bitcoin/system.hpp>

namespace libbitcoin {
namespace database {
namespace storage {

/*
 * Allocate and free memory for storing tuples.
 *
 * TODO:
 *
 * Uses a pool of memory blocks, that are locked by threads when they
 * are writing a tuple in the block.
 *
 * Each block is of fixed size to accomodate type T objects without
 * breaking alignment.
 *
 * Tuples are stored in slots and when tuples are moved to disk, the
 * slot is returned to the list of tuple slots that can be reused for
 * next tuple allocation.
 */
template <typename T>
class memory
  : system::noncopyable
{
public:
    typedef std::shared_ptr<T> memory_ptr;

    // TODO: construct a memory allocator/deallocator
    memory() = default;

    // TODO: destroy the memory allocator; free all allocated memory calling
    // destructor on those objects
    ~memory() = default;

    // Allocate memory for holding a single T
    memory_ptr allocate();
};

}
}
}

#include <bitcoin/database/storage/memory.ipp>

#endif
