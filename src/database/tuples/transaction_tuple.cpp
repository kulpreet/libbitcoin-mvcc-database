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

#include <bitcoin/database/tuples/transaction_tuple.hpp>

namespace libbitcoin {
namespace database {
namespace tuples {

transaction_tuple::transaction_tuple()
  : height(not_found), candidate(0)
{
}

transaction_tuple::operator bool() const
{
    return height != not_found;
}

void transaction_tuple::read_from_delta(transaction_tuple& tuple,
    transaction_tuple_delta& delta)
{
    if (delta.candidate != transaction_tuple_delta::not_set_)
        tuple.candidate = delta.candidate;
    if (delta.position != transaction_tuple_delta::not_set_)
        tuple.position = delta.position;
}

void transaction_tuple::write_to_delta(const transaction_tuple& tuple,
    transaction_tuple_delta& delta)
{
    if (tuple.candidate != transaction_tuple_delta::not_set_)
        delta.candidate = tuple.candidate;
    if (tuple.position != transaction_tuple_delta::not_set_)
        delta.position = tuple.position;
}

} // tuples
} // database
} // libbitcoin
