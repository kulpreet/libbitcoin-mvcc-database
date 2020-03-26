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

#ifndef LIBBITCOIN_MVCC_SLOT_HPP
#define LIBBITCOIN_MVCC_SLOT_HPP

#include <bitcoin/database/storage/raw_block.hpp>

namespace libbitcoin {
namespace database {
namespace storage {

// slot is the sequence of bytes in raw_block where we can store an
// mvcc_tuple or a delta record.
class slot
{
public:

    static const uintptr_t uninitialized = 0;

    /**
     * Constructs an empty tuple slot (uninitialized)
     */
    slot()
      : bytes_(uninitialized)
    {
    }

    /**
     * Construct a tuple slot representing the given offset in the given block
     * @param block the block this slot is in
     * @param offset the offset of this slot in its block
     */
    slot(const raw_block *const block, const uint32_t offset)
        : bytes_(reinterpret_cast<uintptr_t>(block) | offset)
    {
        BITCOIN_ASSERT_MSG(
            !((static_cast<uintptr_t>(BLOCK_SIZE) - 1) &
                ((uintptr_t)block)),
            "Address must be aligned to block size (last bits zero).");
        BITCOIN_ASSERT_MSG(offset < BLOCK_SIZE,
            "Offset must be smaller than block size (to fit in the last bits).");
    }

    // get memory identified by this slot
    uintptr_t get_bytes() const
    {
        return bytes_;
    }

    /**
     * @return ptr to the head of the block
     */
    raw_block *get_block() const
    {
        // Get the first 44 bits as the ptr
        return reinterpret_cast<raw_block *>(bytes_ &
            ~(static_cast<uintptr_t>(BLOCK_SIZE) - 1));
    }

    /**
     * @return offset of the tuple within a block.
     */
    uint32_t get_offset() const
    {
        return static_cast<uint32_t>(bytes_ &
            (static_cast<uintptr_t>(BLOCK_SIZE) - 1));
    }

    /**
     * Checks if this slot is equal to the other.
     * @param other the other slot to be compared.
     * @return true if the slots are equal, false otherwise.
     */
    bool operator==(const slot &other) const
    {
        return bytes_ == other.bytes_;
    }

    /**
     * Checks if this slot is not equal to the other.
     * @param other the other slot to be compared.
     * @return true if the slots are not equal, false otherwise.
     */
    bool operator!=(const slot &other) const
    {
        return bytes_ != other.bytes_;
    }

    operator bool() const
    {
        return bytes_ != uninitialized;
    }

  /**
   * Outputs the slot to the output stream.
   * @param os output stream to be written to.
   * @param slot slot to be output.
   * @return the modified output stream.
   */
  friend std::ostream &operator<<(std::ostream &os, const slot &slot) {
      return os << "block: " << slot.get_block() << ", offset: " << slot.get_offset();
  }

private:
    // Block pointers are always aligned to 1 mb, thus we get 5 free bytes to
    // store the offset.
    uintptr_t bytes_;
};

typedef std::shared_ptr<slot> slot_ptr;

} // namespace storage
} // namespace database
} // namespace libbitcoin

#endif
