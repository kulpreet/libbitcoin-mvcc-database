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
#ifndef LIBBITCOIN_DATABASE_TRANSACTION_MANAGER_HPP
#define LIBBITCOIN_DATABASE_TRANSACTION_MANAGER_HPP

#include <unordered_set>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/transaction_management/spinlatch.hpp>
#include <bitcoin/system.hpp>

#include <bitcoin/database/transaction_management/transaction_context.hpp>

typedef uint64_t timestamp_t;

namespace libbitcoin {
namespace database {

typedef std::unordered_set<timestamp_t> transaction_set;

/// transaction_manager implements a global transaction table and is
/// responsible for starting and commiting transactions.
///
/// The implementation uses a spinlock to avoid heavier weight
/// mutexes.
///
/// TODO: As per Silo generate transaction ids using thread id,
/// or as per terrier, use thread local data, or batch transaction
/// ids. But this can wait till we identify transaction id generation
/// as a bottleneck
class transaction_manager
{
public:
    /// Constructor
    transaction_manager();

    /// Begin a transaction. Caller synchronously waits for a
    /// transaction_context.
    transaction_context begin_transaction();

    /// Commit transaction. Transaction context is released.
    /// Global state transaction table entry for this context
    /// is removed.
    void commit_transaction(transaction_context& context) const;

    void remove_transaction(const transaction_context& context);

    bool is_active(const transaction_context& context) const;

private:
    std::shared_ptr<spinlatch> latch_;

    /// TODO: time_ needs to wrap around. We need to handle that when we get
    // to concurrency control protocols that depend on these timestamps.
    //
    // TODO: For large number of cores, say > 16, we will need to
    // optimise issuing timestamps. Maybe batch them or do a per
    // thread count, and the snapshot system them reads from all threads.
    std::atomic<timestamp_t> time_;
    transaction_set current_transactions_;
};

} // namespace database
} // namespace libbitcoin

#endif
