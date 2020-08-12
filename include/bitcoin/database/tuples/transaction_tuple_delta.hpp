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

#ifndef LIBBITCOIN_MVCC_DATABASE_TRANSACTION_TUPLE_DELTA_HPP
#define LIBBITCOIN_MVCC_DATABASE_TRANSACTION_TUPLE_DELTA_HPP

#include <atomic>
#include <cstddef>

#include <bitcoin/system.hpp>
#include <bitcoin/database/define.hpp>

namespace libbitcoin {
namespace database {
namespace tuples {

using namespace system;

class alignas(8) transaction_tuple_delta {
public:
    // 1 byte
    uint8_t candidate;
    // 2 bytes
    uint16_t position;

    static const uint8_t not_set_ = -1;

    transaction_tuple_delta()
      : candidate(not_set_), position(not_set_)
    {
    }

    bool operator==(transaction_tuple_delta& other)
    {
        return candidate == other.candidate && position == other.position;
    }

    bool operator!=(transaction_tuple_delta& other)
    {
        return candidate != other.candidate  && position != other.position;
    }

};

typedef std::shared_ptr<transaction_tuple_delta> transaction_delta_ptr;

} // namespace tuples
} // namespace database
} // namespace libbitcoin

#endif
