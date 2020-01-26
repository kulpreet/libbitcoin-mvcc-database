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

#ifndef LIBBITCOIN_MVCC_DATABASE_BLOCK_TUPLE_HPP
#define LIBBITCOIN_MVCC_DATABASE_BLOCK_TUPLE_HPP

#include <atomic>
#include <cstddef>

#include <bitcoin/system.hpp>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/storage/memory.hpp>

namespace libbitcoin {
namespace database {
namespace tuples {

using namespace system;

/*
 * Struct to hold block data in memory
 * Attributes ordered for alignment.
 *
 * Note, the sizeof will return 104 bytes. The state field is padded
 * with 7 bytes by the compiler.
 *
 * This results in 7 bytes wasted over 104 bytes, i.e. almost 7%
 * wastage.
 *
 * TODO: Can we reduce this wasted space?
 */
struct block_tuple {
    // header data, 80 bytes
    // 32 bytes
    hash_digest previous_block_hash;
    // 32 bytes
    hash_digest merkle_root;
    // 4 bytes
    uint32_t version;
    // 4 bytes
    uint32_t timestamp;
    // 4 bytes
    uint32_t bits;
    // 4 bytes
    uint32_t nonce;

    // block data, 17 bytes (24 after padding state with 7 bytes)

    // 8 bytes (assumption)
    size_t height;
    // 4 bytes
    uint32_t median_time_past;
    // 4 bytes
    uint32_t checksum;
    // 1 byte
    uint8_t state;
};

typedef std::shared_ptr<block_tuple> block_tuple_ptr;

} // namespace tuples
} // namespace database
} // namespace libbitcoin

#endif
