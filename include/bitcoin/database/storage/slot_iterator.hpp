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

#ifndef LIBBITCOIN_MVCC_SLOT_ITERATOR_HPP
#define LIBBITCOIN_MVCC_SLOT_ITERATOR_HPP

#include <bitcoin/database/storage/raw_block.hpp>
#include <bitcoin/database/storage/slot.hpp>

namespace libbitcoin {
namespace database {
namespace storage {

/**
 * Iterator for all the slots, claimed or otherwise, in the data
 * table. This is useful for sequential scans.
 */
class slot_iterator {
public:
    /**
     * @return reference to the underlying tuple slot
     */
    const slot &operator*() const
    {
        return current_slot_;
    }

    /**
     * @return pointer to the underlying tuple slot
     */
    const slot *operator->() const
    {
        return &current_slot_;
    }

    /**
     * pre-fix increment.
     * @return self-reference after the iterator is advanced
     */
    slot_iterator &operator++();

    /**
     * post-fix increment.
     * @return copy of the iterator equal to this before increment
     */
    slot_iterator operator++(int) {
      slot_iterator copy = *this;
      operator++();
      return copy;
    }

    /**
     * Equality check.
     * @param other other iterator to compare to
     * @return if the two iterators point to the same slot
     */
    bool operator==(const slot_iterator &other) const {
      return current_slot_ == other.current_slot_;
    }

    /**
     * Inequality check.
     * @param other other iterator to compare to
     * @return if the two iterators are not equal
     */
    bool operator!=(const slot_iterator &other) const
    {
        return !this->operator==(other);
    }

   private:
    /**
     * @warning MUST BE CALLED ONLY WHEN CALLER HOLDS LOCK TO THE LIST OF RAW BLOCKS IN THE DATA TABLE
     */
    slot_iterator(raw_block* block, uint32_t offset_in_block)
    {
        current_slot_ = {block, offset_in_block};
    }

    slot current_slot_;
  };

} // namespace storage
} // namespace database
} // namespace libbitcoin

#endif
