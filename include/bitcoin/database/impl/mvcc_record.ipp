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

template <typename tuple, typename delta>
typename mvcc_record<tuple, delta>::delta_mvcc_record*
mvcc_record<tuple, delta>::no_next = new delta_mvcc_record();

template <typename tuple, typename delta>
const typename mvcc_record<tuple, delta>::tuple_ptr
mvcc_record<tuple, delta>::not_found = std::make_shared<tuple>();

// Construct the master record for mvcc list.
// Set it so that it is locked by creating tx context.
// Set begin timestamp is set to passed context's tx id
// This tuple is not yet "installed".
template <typename tuple, typename delta>
mvcc_record<tuple, delta>::mvcc_record(
    const transaction_context& tx_context)
    : txn_id_(tx_context.get_timestamp()),
      read_timestamp_(none_read), begin_timestamp_(tx_context.get_timestamp()),
      end_timestamp_(infinity), data_(tuple()), next_(no_next)
{
}

template <typename tuple, typename delta>
bool mvcc_record<tuple, delta>::get_latch_for_write(
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

template <typename tuple, typename delta>
void mvcc_record<tuple, delta>::write_to(mvcc_record<tuple, delta>* to,
    const transaction_context& context) const
{
    BITCOIN_ASSERT_MSG(to->is_latched_by(context),
        "Before writing to memory, get a write latch on it");
    to->read_timestamp_ = read_timestamp_;
    to->begin_timestamp_ = begin_timestamp_;
    to->end_timestamp_ = end_timestamp_;
    to->data_ = data_;
    to->next_ = next_;
}

template <typename tuple, typename delta>
bool mvcc_record<tuple, delta>::release_latch(
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

template <typename tuple, typename delta>
typename mvcc_record<tuple, delta>::tuple_ptr
mvcc_record<tuple, delta>::read_record(
    const transaction_context &context, void (*reader)(tuple&, delta&))
{
    if (!is_visible(context)) {
        return not_found;
    }

    tuple_ptr result = std::make_shared<tuple>(data_);
    set_read_timestamp(context);

    for (auto delta_record = begin(); delta_record != end(); delta_record++) {
        if ((*delta_record)->can_read(context)) {
            reader(*result, delta_record->get_data());
            delta_record->set_read_timestamp(context);
        } else {
            return result;
        }
    }

    return result;
}

// Uses MVTO protocol
template <typename tuple, typename delta>
bool mvcc_record<tuple, delta>::can_read(
    const transaction_context &context)
{
    return read_timestamp_ <= context.get_timestamp();
}

// Uses MVTO protocol
template <typename tuple, typename delta>
bool mvcc_record<tuple, delta>::is_visible(
    const transaction_context &context)
{
    auto timestamp = context.get_timestamp();

    // No write lock held by any other transaction
    auto old_tid = txn_id_.load();
    if (old_tid != not_locked && old_tid != timestamp) {
        return false;
    }

    // context.timestamp is greater than begin ts
    if (timestamp < begin_timestamp_) {
        return false;
    }

    return true;
}

template <typename tuple, typename delta>
bool mvcc_record<tuple, delta>::is_latched_by(
    const transaction_context& context)
{
    return txn_id_.load() == context.get_timestamp();
}

template <typename tuple, typename delta>
typename mvcc_record<tuple, delta>::delta_mvcc_record_ptr
mvcc_record<tuple, delta>::allocate_next(
    const transaction_context& context)
{
    // MVTO: latch this record before creating the next version
    get_latch_for_write(context);
    return std::make_shared<delta_mvcc_record>(context);
}

template <typename tuple, typename delta>
bool mvcc_record<tuple, delta>::install(
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
    return true;
}

template <typename tuple, typename delta>
bool mvcc_record<tuple, delta>::commit(
    const transaction_context &context)
{
    return commit(context, infinity);
}

template <typename tuple, typename delta>
bool mvcc_record<tuple, delta>::commit(
    const transaction_context &context, const timestamp_t ts)
{
    BITCOIN_ASSERT_MSG(!is_latched_by(context),
        "Trying to install a version without latching it first");

    end_timestamp_ = ts;
    return release_latch(context);
}

template <typename tuple, typename delta>
void mvcc_record<tuple, delta>::install_next_version(
    delta_mvcc_record_ptr delta_record, const transaction_context& context)
{
    install_next_version(delta_record.get(), context);
}

template <typename tuple, typename delta>
void mvcc_record<tuple, delta>::install_next_version(
    delta_mvcc_record* delta_record, const transaction_context& context)
{
    // install delta
    if (!delta_record->install(context)) {
        return;
    }

    // set end ts for this
    end_timestamp_ = context.get_timestamp();

    // set next to point to next delta record
    next_ = delta_record;
}

template <typename tuple, typename delta>
typename mvcc_record<tuple, delta>::iterator
mvcc_record<tuple, delta>::begin() const
{
    return { next_ };
}

template <typename tuple, typename delta>
typename mvcc_record<tuple, delta>::iterator
mvcc_record<tuple, delta>::end() const
{
    return { no_next };
}

template <typename tuple, typename delta>
typename mvcc_record<tuple, delta>::delta_mvcc_record*
mvcc_record<tuple, delta>::get_next() const
{
    return next_;
}

template <typename tuple, typename delta>
mvcc_column mvcc_record<tuple, delta>::get_read_timestamp() const
{
    return read_timestamp_;
}

template <typename tuple, typename delta>
void mvcc_record<tuple, delta>::set_read_timestamp(const transaction_context& context)
{
    read_timestamp_ = context.get_timestamp();
}

template <typename tuple, typename delta>
mvcc_column mvcc_record<tuple, delta>::get_begin_timestamp() const
{
    return begin_timestamp_;
}

template <typename tuple, typename delta>
mvcc_column mvcc_record<tuple, delta>::get_end_timestamp() const
{
    return end_timestamp_;
}

template <typename tuple, typename delta>
tuple& mvcc_record<tuple, delta>::get_data()
{
    return data_;
}

// block delta tuple
template class mvcc_record<block_tuple_delta, block_tuple_delta>;
typedef mvcc_record<block_tuple_delta, block_tuple_delta> block_delta_mvcc_record;

// block tuple wrapped in mvcc record
template class mvcc_record<block_tuple, block_tuple_delta>;
typedef mvcc_record<block_tuple, block_tuple_delta> block_mvcc_record;

} // database
} // libbitcoin
} // namespace tuples

#endif
