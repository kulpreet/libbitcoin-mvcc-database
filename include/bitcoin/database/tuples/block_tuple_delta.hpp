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

#ifndef LIBBITCOIN_MVCC_DATABASE_BLOCK_TUPLE_DELTA_HPP
#define LIBBITCOIN_MVCC_DATABASE_BLOCK_TUPLE_DELTA_HPP

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
 * Struct to hold old values. We are following the N2O order in our
 * delta table.
 */
struct block_tuple_delta {
    // 1 byte
    uint8_t state;
};

} // namespace tuples
} // namespace database
} // namespace libbitcoin

#endif
