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

#include <bitcoin/database/tuples/block_tuple.hpp>

namespace libbitcoin {
namespace database {
namespace tuples {

block_tuple::block_tuple()
  : height(not_found)
{
}

block_tuple::operator bool() const
{
    return height != not_found;
}

void block_tuple::read_from_delta(block_tuple& tuple,
    block_delta_ptr delta)
{
    tuple.state = delta->state;
}

void block_tuple::write_to_delta(const block_tuple& tuple,
    block_delta_ptr delta)
{
    delta->state = tuple.state;
}

} // tuples
} // database
} // libbitcoin
