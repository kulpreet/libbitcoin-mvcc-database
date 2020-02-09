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

#ifndef LIBBITCOIN_MVCC_DATABASE_VERSIONS_ITERATOR_HPP
#define LIBBITCOIN_MVCC_DATABASE_VERSIONS_ITERATOR_HPP

#include <bitcoin/database/transaction_management/transaction_context.hpp>

namespace libbitcoin {
namespace database {
namespace tuples {

// Template class for version chain iterator.
//
// Instantiate using a delta_mvcc_record_ptr from delta mvcc record.
template <typename delta>
class delta_iterator
{
public:
    // std::iterator_traits
    //-------------------------------------------------------------------------

    typedef delta pointer;
    typedef delta reference;
    typedef delta value_type;
    typedef ptrdiff_t difference_type;
    typedef std::forward_iterator_tag iterator_category;
    typedef delta_iterator iterator;
    typedef delta_iterator const_iterator;

    // Constructors.
    //-------------------------------------------------------------------------

    delta_iterator(delta);

    // Operators.
    //-------------------------------------------------------------------------

    pointer operator->() const;
    reference operator*() const;
    delta_iterator& operator++();
    delta_iterator operator++(int);
    bool operator==(const delta_iterator& other) const;
    bool operator!=(const delta_iterator& other) const;

private:
    delta delta_record_;
};

} // namespace tuples
} // namespace database
} // namespace libbitcoin

#endif
