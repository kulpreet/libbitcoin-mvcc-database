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

#include <bitcoin/database/tuples/delta_iterator.hpp>

namespace libbitcoin {
namespace database {
namespace tuples {

using namespace bc::system;
using namespace bc::system::chain;
using namespace bc::database::tuples;

template<typename delta>
delta_iterator<delta>::delta_iterator(delta record)
    :delta_record_(record)
{
}

template<typename delta>
typename delta_iterator<delta>::pointer delta_iterator<delta>::operator->() const
{
    return delta_record_;
}

template<typename delta>
typename delta_iterator<delta>::reference delta_iterator<delta>::operator*() const
{
    return delta_record_;
}

template<typename delta>
typename delta_iterator<delta>::iterator& delta_iterator<delta>::operator++()
{
    delta_record_ = delta_record_.get_next();
    return *this;
}

template <typename delta>
typename delta_iterator<delta>::iterator delta_iterator<delta>::operator++(int)
{
  auto it = *this;
  delta_record_ = delta_record_.get_next();
  return it;
}

// Simple pointer equality
template <typename delta>
bool delta_iterator<delta>::operator==(const delta_iterator &other) const
{
  return this == other;
}

// Simple pointer inequality
template <typename delta>
bool delta_iterator<delta>::operator!=(const delta_iterator &other) const
{
  return this != other;
}

} // tuples
} // database
} // libbiitcoin

