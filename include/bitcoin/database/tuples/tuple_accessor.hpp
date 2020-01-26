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

#ifndef LIBBITCOIN_MVCC_DATABASE_TUPLE_ACEESSOR_HPP
#define LIBBITCOIN_MVCC_DATABASE_TUPLE_ACCESSOR_HPP

#include <atomic>
#include <cstddef>

#include <bitcoin/system.hpp>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/transaction_management/transaction_context.hpp>
#include <bitcoin/database/storage/memory.hpp>

namespace libbitcoin {
namespace database {
namespace tuples {

using namespace system;

/*
 * Template to read and write attributes for a tuple.
 * Use MVTO based concurrency control.
 */
template<typename tuple, typename delta>
class tuple_accessor{
public:
    // Given a pointer to a master tuple and transaction context,
    // return a ptr to tuple that contains attribute values corresponding
    // to what the transaction context can read.
    // Return nullptr if nothing can be fetched to match the
    // conditions. Caller should check for nullptr return.
    tuple fetch(const transaction_context& context, const tuple master);

    // Given a transaction context, create a delta storage record.
    // The attributes for the delta record will be set by caller.
    // The transaction context will save reference to delta record for later use
    delta create_delta_for_update(const transaction_context& context,
        const tuple master);

    // Commit delta record. Called by transaction.
    bool commit_delta(const transaction_context &context, tuple master,
        delta delta);
};

} // namespace tuples
} // namespace database
} // namespace libbitcoin

#endif
