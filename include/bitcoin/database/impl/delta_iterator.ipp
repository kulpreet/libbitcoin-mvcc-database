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

#ifndef LIBBITCOIN_MVCC_DATABASE_DELTA_ITERATOR_IPP
#define LIBBITCOIN_MVCC_DATABASE_DELTA_ITERATOR_IPP

#include <bitcoin/database/tuples/delta_iterator.hpp>

namespace libbitcoin {
namespace database {
namespace tuples {

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
    delta_record_ = delta_record_->get_next();
    return *this;
}

template <typename delta>
typename delta_iterator<delta>::iterator delta_iterator<delta>::operator++(int)
{
  auto it = *this;
  delta_record_ = delta_record_->get_next();
  return it;
}

// Only used to check end() and therefore checks state to match not_found_
template <typename delta>
bool delta_iterator<delta>::operator==(const delta_iterator& other) const
{
    return (**this)->get_data() == (*other)->get_data();
}

template <typename delta>
bool delta_iterator<delta>::operator!=(const delta_iterator& other) const
{
    return !(*this == other);
}


} // tuples
} // database
} // libbiitcoin

#endif
