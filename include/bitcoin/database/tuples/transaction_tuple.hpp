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

#ifndef LIBBITCOIN_MVCC_DATABASE_TRANSACTION_TUPLE_HPP
#define LIBBITCOIN_MVCC_DATABASE_TRANSACTION_TUPLE_HPP

#include <atomic>
#include <cstddef>

#include <bitcoin/system.hpp>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/tuples/transaction_tuple_delta.hpp>

namespace libbitcoin {
namespace database {
namespace tuples {

using namespace system;

/*
 * Struct to hold transaction data in memory
 * Attributes ordered for alignment.
 *
 */
class transaction_tuple {
public:

    static const uint32_t not_found = -1;

    transaction_tuple();

    operator bool() const;

    static void read_from_delta(transaction_tuple&, transaction_tuple_delta&);

    static void write_to_delta(const transaction_tuple&, transaction_tuple_delta&);

//-------------------------------------------------------------
// data stored

    // Transaction (without inputs and outputs) 20 bytes.
    // Inputs and outputs are varlen so not stored here.
    // Even the input and ouput counts are varlen so they are not
    // stored here either.

    // 4 bytes
    uint32_t height;
    // 4 bytes
    uint32_t median_time_past;
    // 4 bytes
    uint32_t locktime;
    // 4 bytes
    uint32_t version;
    // 2 bytes
    uint16_t position;
    // 1 byte
    uint8_t candidate;
    // 1 byte
    uint8_t witness_flag;
//-------------------------------------------------------------

};

typedef std::shared_ptr<transaction_tuple> transaction_tuple_ptr;

} // namespace tuples
} // namespace database
} // namespace libbitcoin

#endif
