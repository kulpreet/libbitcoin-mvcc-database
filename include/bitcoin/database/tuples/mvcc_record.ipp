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

// Construct the master record for mvcc list
// Set it so that it is not locked
// Set begin timestamp is set to passed context's tx id
// This tuple is not yet "installed"
template <typename tuple, typename delta_mvcc_record>
mvcc_record<tuple, delta_mvcc_record>::mvcc_record(
    const transaction_context &tx_context)
  : txn_id_(not_locked), data_(tuple()),
    read_timestamp_(infinity), begin_timestamp_(tx_context.get_timestamp()),
    end_timestamp_(infinity)
{
}

template <typename tuple, typename delta_mvcc_record>
delta_mvcc_record mvcc_record<tuple, delta_mvcc_record>::next_version() {
  return next_;
}

} // namespace tuples
} // namespace database
} // namespace libbitcoin

#endif
