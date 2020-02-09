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

#ifndef LIBBITCOIN_MVCC_DATABASE_MVCC_RECORD_IPP
#define LIBBITCOIN_MVCC_DATABASE_MVCC_RECORD_IPP

#include <atomic>

#include <bitcoin/database/tuples/mvcc_record.hpp>

namespace libbitcoin {
namespace database {
namespace tuples {

// Construct the master record for mvcc list.
// Set it so that it is locked by creating tx context.
// Set begin timestamp is set to passed context's tx id
// This tuple is not yet "installed".
template <typename tuple, typename delta, typename reader, typename writer>
mvcc_record<tuple, delta, reader, writer>::mvcc_record(
    const transaction_context& tx_context)
  : txn_id_(tx_context.get_timestamp()), data_(tuple()),
    read_timestamp_(none_read), begin_timestamp_(tx_context.get_timestamp()),
    end_timestamp_(infinity)
{
}

template <typename tuple, typename delta, typename reader, typename writer>
mvcc_record<tuple, delta, reader, writer>::mvcc_record(
    const tuple data, const transaction_context& tx_context)
  : txn_id_(tx_context.get_timestamp()), data_(data),
    read_timestamp_(none_read), begin_timestamp_(tx_context.get_timestamp()),
    end_timestamp_(infinity)
{
}

template <typename tuple, typename delta, typename reader, typename writer>
bool mvcc_record<tuple, delta, reader, writer>::get_latch_for_write(
    const transaction_context& context)
{
    auto old_tid = txn_id_.load();
    auto tid = context.get_timestamp();

    // already locked by tid
    if (old_tid == tid)
        return true;

    // try to get the lock
    auto unlocked = not_locked;
    return txn_id_.compare_exchange_strong(unlocked, tid);
}

template <typename tuple, typename delta, typename reader, typename writer>
bool mvcc_record<tuple, delta, reader, writer>::release_latch(
    const transaction_context& context)
{
    auto old_tid = txn_id_.load();
    auto tid = context.get_timestamp();

    // locked by tid, try to release the latch
    if (old_tid == tid)
        return txn_id_.compare_exchange_strong(tid, not_locked);

    // Locked by another txn or already unlocked
    return false;
}

template <typename tuple, typename delta, typename reader, typename writer>
tuple mvcc_record<tuple, delta, reader, writer>::read_record(
    const transaction_context &context)
{
    if (!is_visible(context)) {
        return nullptr;
    }

    tuple result{data_};

    // for (auto delta_record = versions.begin(); delta_record != versions.end(); delta_record++) {
    //     if (delta_record.is_visible(context)) {
    //         reader(result, delta_record);
    //     } else {
    //         return result;
    //     }
    // }

    return result;
}

// Uses MVTO protocol
template <typename tuple, typename delta, typename reader, typename writer>
bool mvcc_record<tuple, delta, reader, writer>::is_visible(
    const transaction_context &context)
{
    auto timestamp = context.get_timestamp();

    // No write lock held by any other transaction
    auto old_tid = txn_id_.load();
    if (old_tid != not_locked && old_tid != timestamp) {
        return false;
    }

    // context.timestamp is between begin and end ts
    if (timestamp < begin_timestamp_ || timestamp > end_timestamp_) {
        return false;
    }

    return true;
}

template <typename tuple, typename delta, typename reader, typename writer>
bool mvcc_record<tuple, delta, reader, writer>::is_latched_by(
    const transaction_context& context)
{
    return txn_id_.load() == context.get_timestamp();
}

template <typename tuple, typename delta, typename reader, typename writer>
typename mvcc_record<tuple, delta, reader, writer>::delta_mvcc_record_ptr
mvcc_record<tuple, delta, reader, writer>::allocate_next(
    const delta_data_t data, const transaction_context& context)
{
    // MVTO: latch this record before creating the next version
    get_latch_for_write(context);
    return std::make_shared<delta_mvcc_record>(data, context);
}

template <typename tuple, typename delta, typename reader, typename writer>
bool mvcc_record<tuple, delta, reader, writer>::install(
    const transaction_context &context)
{
    BITCOIN_ASSERT_MSG(!is_latched_by(context),
        "Trying to install a version without latching it first");
    auto timestamp = context.get_timestamp();

    // install fails if txn id is not the same as this context
    // timestamp
    if (timestamp != txn_id_.load()) {
        return false;
    }

    // set end ts
    end_timestamp_ = timestamp;

    // release latch
    return txn_id_.compare_exchange_strong(timestamp, not_locked);
}

template <typename tuple, typename delta, typename reader, typename writer>
bool mvcc_record<tuple, delta, reader, writer>::install_next_version(
    delta_mvcc_record_ptr delta_record, const transaction_context& context)
{
    // install delta
    if (!delta_record.install(context)) {
        return false;
    }
    // set end ts for this
    end_timestamp_ = context.get_timestamp();
    // set next to point to next delta record
    next_ = delta_record;
    // release latch on this
    return release_latch(context);
}

} // namespace tuples
} // namespace database
} // namespace libbitcoin

#endif
