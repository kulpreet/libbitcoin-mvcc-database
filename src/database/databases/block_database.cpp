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

#include <bitcoin/database/databases/block_database.hpp>
#include <bitcoin/database/tuples/block_tuple.hpp>

namespace libbitcoin {
namespace database {

using namespace bc::system;
using namespace bc::system::chain;
using namespace bc::database::tuples;

block_tuple_ptr block_database::store(const system::chain::header& header,
    const size_t height, const uint32_t median_time_past,
    const uint32_t checksum, const uint8_t state)
{
}

} // namespace database
} // namespace libbitcoin
