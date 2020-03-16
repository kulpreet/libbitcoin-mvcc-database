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

#ifndef LIBBITCOIN_MVCC_RAW_BITMAP_HPP
#define LIBBITCOIN_MVCC_RAW_BITMAP_HPP

#include <cstring>
#include <memory>

#ifndef BYTE_SIZE
#define BYTE_SIZE 8U
#endif

// Some platforms would have already defined the macro. But its
// presence is not standard and thus not portable. Pretty sure this is
// always 8 bits. If not, consider getting a new machine, and
// preferably not from another dimension :)
static_assert(BYTE_SIZE == 8U, "BYTE_SIZE should be set to 8!");

// n must be [0, 7], all 0 except for 1 on the nth bit in LSB order.
#define LSB_ONE_HOT_MASK(n) (1U << n)
// n must be [0, 7], all 1 except for 0 on the nth bit in LSB order
#define LSB_ONE_COLD_MASK(n) (0xFF - LSB_ONE_HOT_MASK(n))

namespace libbitcoin {
namespace database {
namespace container {

/**
 * A raw_bitmap is a bitmap that does not have the compile-time
 * information about sizes, because we expect it to be reinterpreted
 * from raw memory bytes.
 *
 * Therefore, you should never construct an instance of a
 * raw_bitmap. Reinterpret an existing block of memory that you know
 * will be a valid bitmap.
 *
 * Use @see common::BitmapSize to get the correct size for a bitmap of
 * n elements. Beware that because the size information is lost at
 * compile time, there is ABSOLUTELY no bounds check and you have to
 * rely on programming discipline to ensure safe access.
 *
 * For easy initialization in tests and such, use the static Allocate
 * and Deallocate methods
 */
class raw_bitmap {
public:

    // Disallow creating and deleting the bitmap. It can only be
    // interpreted as a bitmpa from already allocated memory.
    raw_bitmap() = delete;
    ~raw_bitmap() = delete;

    /**
     * @param n number of elements in the bitmap
     * @return the size of the bitmap holding the given number of
     * elements, in bytes.
     */
    static constexpr uint32_t size_in_bytes(uint32_t n)
    {
        return n % BYTE_SIZE == 0 ? n / BYTE_SIZE : n / BYTE_SIZE + 1;
    }

    /**
     * Allocates a new raw_bitmap of size num_bits.
     * Up to the caller to call deallocate on its return value.
     * @param num_bits number of bits (elements to represent) in the bitmap.
     * @return ptr to new raw_bitmap.
     */
    static raw_bitmap *allocate(const uint32_t num_bits)
    {
        auto size = size_in_bytes(num_bits);
        auto *result = new uint8_t[size];
        std::memset(result, 0, size);
        return reinterpret_cast<raw_bitmap *>(result);
    }

    /**
     * Deallocates a raw_bitmap. Only call on pointers given out by
     * allocate
     * @param map the map to deallocate
     */
    static void deallocate(raw_bitmap *const map)
    {
        delete[] reinterpret_cast<uint8_t *>(map);
    }

    /**
     * Test the bit value at the given position
     * @param pos position to test
     * @return true if 1, false if 0
     */
    bool test(const uint32_t pos) const
    {
        return static_cast<bool>(
            bits_[pos / BYTE_SIZE] & LSB_ONE_HOT_MASK(pos % BYTE_SIZE));
    }

    /**
     * Test the bit value at the given position
     * @param pos position to test
     * @return true if 1, false if 0
     */
    bool operator[](const uint32_t pos) const
    {
        return test(pos);
    }

    /**
     * Sets the bit value at position to be true.
     * @param pos position to test
     * @param val value to set to
     * @return self-reference for chaining
     */
    raw_bitmap &set(const uint32_t pos, const bool val)
    {
        if (val)
            bits_[pos / BYTE_SIZE] |=
                static_cast<uint8_t>(LSB_ONE_HOT_MASK(pos % BYTE_SIZE));
        else
            bits_[pos / BYTE_SIZE] &=
                static_cast<uint8_t>(LSB_ONE_COLD_MASK(pos % BYTE_SIZE));
        return *this;
    }

    /**
     * @brief Flip the bit
     * @param pos the position of the bit to flip
     * @return self-reference for chaining
     */
    raw_bitmap &flip(const uint32_t pos)
    {
        bits_[pos / BYTE_SIZE] ^= static_cast<uint8_t>(LSB_ONE_HOT_MASK(pos % BYTE_SIZE));
        return *this;
    }

    /**
     * Clears the bitmap by setting bits to 0.
     * @param num_bits number of bits to clear. This should be equal
     * to the number of elements of the entire bitmap or
     * unintended elements may be cleared
     */
    void clear(const uint32_t num_bits)
    {
        auto size = size_in_bytes(num_bits);
        std::memset(bits_, 0, size);
    }

private:
    uint8_t bits_[0];
};

// WARNING: DO NOT CHANGE THE CLASS LAYOUT OF raw_bitmap. The
// correctness of our storage code depends in this class having this
// exact layout. Changes include marking a function as virtual, as
// that adds a Vtable to the class layout,
static_assert(sizeof(raw_bitmap) == 0, "Unexpected raw_bitmap layout!");


} // container
} // database
} // container

#endif
