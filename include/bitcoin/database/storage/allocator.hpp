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

#ifndef LIBBITCOIN_MVCC_DATABASE_ALLOCATOR_HPP
#define LIBBITCOIN_MVCC_DATABASE_ALLOCATOR_HPP

#include <bitcoin/database/define.hpp>
#include <cstddef>

namespace libbitcoin {
namespace database {
namespace storage {

/**
 * Static utility class for more advanced memory allocation behavior
 */
struct allocation_util {
  allocation_util() = delete;

  /**
   * Allocates a chunk of memory whose start address is guaranteed to be aligned to 8 bytes
   * @param byte_size size of the memory chunk to allocate, in bytes
   * @return allocated memory pointer
   */
  static uint8_t *allocate_aligned(uint64_t byte_size) {
    // This is basically allocating the chunk as a 64-bit array, which forces c++ to give back to us
    // 8 byte-aligned addresses. + 7 / 8 is equivalent to padding up the nearest 8-byte size. We
    // use this hack instead of std::aligned_alloc because calling delete on it does not make ASAN
    // happy on Linux + GCC, and calling std::free on pointers obtained from new is undefined behavior.
    // Having to support two paradigms when we liberally use byte * throughout the codebase is a
    // maintainability nightmare.
    return reinterpret_cast<uint8_t *>(new uint64_t[(byte_size + 7) / 8]);
  }

  /**
   * Allocates an array of elements that start at an 8-byte aligned address
   * @tparam T type of element
   * @param size number of elements to allocate
   * @return allocated memory pointer
   */
  template <class T>
  static T *allocate_aligned(uint32_t size) {
    return reinterpret_cast<T *>(allocate_aligned(size * sizeof(T)));
  }
};

/**
 * Allocator that allocates and destroys a byte array. Memory location returned by this default allocator is
 * not zeroed-out. The address returned is guaranteed to be aligned to 8 bytes.
 * @tparam T object whose size determines the byte array size.
 */
template <typename T>
class byte_aligned_allocator {
 public:
  /**
   * Allocates a new byte array sized to hold a T.
   * @return a pointer to the byte array allocated.
   */
  T *allocate()
    {
        auto *result = reinterpret_cast<T *>(allocation_util::allocate_aligned(sizeof(T)));
        reuse(result);
        return result;
    }

  /**
   * Reuse a reused chunk of memory to be handed out again
   * @param reused memory location, possibly filled with junk bytes
   */
  void reuse(T *const reused)
    {
    }

  /**
   * Deletes the byte array.
   * @param ptr pointer to the byte array to be deleted.
   */
  void deallocate(T *const ptr)
    {
        delete[] reinterpret_cast<uint8_t *>(ptr);
    }
};

} // namespace storage
} // namespace database
} // namespace libbitcoin

#endif
