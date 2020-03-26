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

#ifndef LIBBITCOIN_MVCC_DATABASE_MVTO_ACCESSOR_IPP
#define LIBBITCOIN_MVCC_DATABASE_MVTO_ACCESSOR_IPP

#include <bitcoin/database/mvto/accessor.hpp>

namespace libbitcoin {
namespace database {
namespace mvto {

template <typename mvcc_tuple, typename mvcc_delta>
accessor<mvcc_tuple, mvcc_delta>::accessor(tuple_store_ptr tuple_store,
    delta_store_ptr delta_store)
    : tuple_store_(tuple_store), delta_store_(delta_store)
{
}

template <typename mvcc_tuple, typename mvcc_delta>
slot accessor<mvcc_tuple, mvcc_delta>::put(transaction_context& context,
    typename mvcc_tuple::tuple_ptr tuple)
{
    const mvcc_tuple record(context, tuple);
    auto record_slot = tuple_store_->insert(context, record);
    auto record_bytes = record_slot.get_bytes();
    auto record_ptr = reinterpret_cast<mvcc_tuple*>(record_bytes);

    if (!record_ptr->install(context))
    {
        return slot{};
    }

    context.register_commit_action([record_ptr, context]()
    {
        // commit to context timestamp
        record_ptr->commit(context, context.get_timestamp());
    });

    auto end_ts = record_ptr->get_end_timestamp();
    auto next = record_ptr->get_next();
    context.register_abort_action([record_ptr, context, end_ts, next]()
    {
        // reset next
        record_ptr->set_next(next);
        // reset end timestamp and release latch, using commit
        record_ptr->commit(context, end_ts);
    });
    return record_slot;
}

template <typename mvcc_tuple, typename mvcc_delta>
bool accessor<mvcc_tuple, mvcc_delta>::update(transaction_context& context,
    slot& slot, mvcc_tuple& tuple)
{
    return false;
}

template <typename mvcc_tuple, typename mvcc_delta>
typename mvcc_tuple::tuple_ptr
accessor<mvcc_tuple, mvcc_delta>::get(transaction_context& context,
    slot& from, typename mvcc_tuple::reader reader)
{
    return tuple_store_->read(from, context, reader);
}

} // namespace libbitcoin
} // namespace database
} // namespace mvto

#endif
