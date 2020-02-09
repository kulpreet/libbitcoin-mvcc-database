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
#include <bitcoin/database/tuples/delta_iterator.hpp>

namespace libbitcoin {
namespace database {
namespace tuples {

typedef uint64_t mvcc_column;

// Define infinity to be -1. Might have to change this.
const uint64_t infinity = -1;

// not_locked used as the sentinel to mark record is not locked.
const uint64_t not_locked = 0;

// No one has read this version yet.
const uint64_t none_read = 0;

// Template for providing MVCC record keeping for tuple
// Each record containts MVCC data and a pointed to next
// version record.
// tuple will be pointer to block_tuple, utxo_tuple, etc.
// delta is a pointer to the delta data struct
// reader and writer are functors
template <typename tuple, typename delta, typename reader = void,
          typename writer = void>
class mvcc_record {
public:
    typedef tuple master_data_t;
    typedef delta delta_data_t;
    typedef mvcc_record<delta_data_t, delta_data_t> delta_mvcc_record;
    typedef std::shared_ptr<delta_mvcc_record> delta_mvcc_record_ptr;

    // constructors
    mvcc_record(const transaction_context&);
    mvcc_record(const tuple, const transaction_context&);

    // return a tuple with attributes set according to the reader
    // function. Finds version that is readable by context and sets
    // the value appropriate in tuple. If can't read any version, then
    // returns nullptr - the caller should check for this.
    tuple read_record(const transaction_context&);

    // sets up a new version using the transaction context and the
    // writer. This will later be installed.
    //
    // MVTO: Can only be called by a transaction that already has a
    // latch on the latest version and the new tid is greater than
    // readts
    // latch this record by setting txn_id_ from context
    // Allocate delta record with:
    // - tid locked to context timestamp
    // - begin set to context timestamp
    // - end ts to infinity
    // - read ts to 0, no one has read it yet
    // Does not install the allocated record.
    delta_mvcc_record_ptr allocate_next(const delta_data_t, const transaction_context&);

    // install this version, return true on success.
    bool install(const transaction_context&);

    // install the next record from this version, return true on
    // success.
    bool install_next_version(delta_mvcc_record_ptr, const transaction_context&);

    // Get a latch on the record
    // TODO - do we need to specify memory order?
    bool get_latch_for_write(const transaction_context&);

    // release the latch on this record
    bool release_latch(const transaction_context&);

    mvcc_column get_read_timestamp() { return read_timestamp_; }

    mvcc_column get_begin_timestamp() { return begin_timestamp_; }

    mvcc_column get_end_timestamp() { return end_timestamp_; }

    tuple get_data() { return data_; }

    delta_mvcc_record_ptr get_next() { return next_; }

    // Iterator definition and operators
    delta_iterator<delta_mvcc_record_ptr> begin() { return next_; }
    delta_iterator<delta_mvcc_record_ptr> end() { return nullptr; }

private:
    // Compare and swap on txn_id_ "installs" the new version
    // txn_id_ acts as a local latch on this record.
    std::atomic<timestamp_t> txn_id_;

    // read_timestamp_ tracks the largest timestamp of transactions
    // reading from the record
    mvcc_column read_timestamp_;

    // begin and end timestamps determine which transactions can read
    // this version
    mvcc_column begin_timestamp_;
    mvcc_column end_timestamp_;

    // data is a tuple pointer
    tuple data_;

    // next_ points to the next version
    delta_mvcc_record_ptr next_;

    // returns true if this tuple is visible to transaction
    bool is_visible(const transaction_context&);

    // returns true if this tuple is latched by context
    bool is_latched_by(const transaction_context&);
};

} // namespace tuples
} // namespace database
} // namespace libbitcoin

#include <bitcoin/database/tuples/mvcc_record.ipp>

#endif
