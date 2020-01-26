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

#ifndef LIBBITCOIN_MVCC_DATABASE_MVCC_RECORD_HPP
#define LIBBITCOIN_MVCC_DATABASE_MVCC_RECORD_HPP

#include <bitcoin/database/transaction_management/transaction_context.hpp>

namespace libbitcoin {
namespace database {
namespace tuples {

typedef uint64_t mvcc_column;

// Define infinity to be -1. Might have to change this.
const uint64_t infinity = -1;

// not_locked used as the sentinel to mark record is not locked
const uint64_t not_locked = 0;

// Template for providing MVCC record keeping for tuple
// Each record containts MVCC data and a pointed to next
// version record.
// tuple will be pointer to block_tuple, utxo_tuple, etc.
// Both tuple and delta are pointers.
template<typename tuple, typename delta_mvcc_record>
class mvcc_record {
public:
    // constructor
    mvcc_record(const transaction_context&);

    // get next version
    delta_mvcc_record next_version();

    mvcc_column get_txn_id() {
        return txn_id_;
    }

    mvcc_column get_read_timestamp() {
        return read_timestamp_;
    }

    mvcc_column get_begin_timestamp() {
        return begin_timestamp_;
    }

    mvcc_column get_end_timestamp() {
        return end_timestamp_;
    }

    tuple get_data() {
        return data_;
    }

    delta_mvcc_record get_next() {
        return next_;
    }

private:
    // Compare and swap on txn_id_ "installs" the new version
    // txn_id_ acts as a local latch on this record.
    std::atomic<mvcc_column> txn_id_;

    // read_timestamp_ tracks the largest timestamp of transactions
    // reading from the record
    mvcc_column read_timestamp_;

    // begin and end timestamps determine which transactions can read
    // this version
    mvcc_column begin_timestamp_;
    mvcc_column end_timestamp_;

    // data is a tuple pointer
    tuple data_;

    // The next record is always delta
    delta_mvcc_record next_;
};

} // namespace tuples
} // namespace database
} // namespace libbitcoin

#include <bitcoin/database/tuples/mvcc_record.ipp>

#endif
