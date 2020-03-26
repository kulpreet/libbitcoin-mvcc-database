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

#ifndef LIBBITCOIN_MVCC_DATABASE_MVTO_ACCESSOR_HPP
#define LIBBITCOIN_MVCC_DATABASE_MVTO_ACCESSOR_HPP

#include <cstddef>

#include <bitcoin/database/define.hpp>
#include <bitcoin/database/transaction_management/transaction_context.hpp>
#include <bitcoin/database/storage/slot.hpp>
#include <bitcoin/database/storage/slot_iterator.hpp>
#include <bitcoin/database/storage/storage.hpp>

namespace libbitcoin {
namespace database {
namespace mvto {

using namespace bc::database::storage;

template<typename mvcc_tuple, typename mvcc_delta>
class accessor
{
public:
    typedef store<mvcc_tuple> tuple_store;
    typedef std::shared_ptr<store<mvcc_tuple>> tuple_store_ptr;

    typedef store<mvcc_delta> delta_store;
    typedef std::shared_ptr<store<mvcc_delta>> delta_store_ptr;

    accessor(tuple_store_ptr, delta_store_ptr);

    slot put(transaction_context&, typename mvcc_tuple::tuple_ptr);

    // write mvcc_tuple as an update to head pointed by slot.
    // creates a new version
    bool update(transaction_context&, slot&, mvcc_tuple&);

    // Reads from slot, following all the versions to return final
    // resolved value
    bool get(transaction_context&, slot&, mvcc_tuple*);

private:
    tuple_store_ptr tuple_store_;
    delta_store_ptr delta_store_;
};

} // namespace mvto
} // namespace database
} // namespace libbitcoin

#include <bitcoin/database/impl/accessor.ipp>

#endif
