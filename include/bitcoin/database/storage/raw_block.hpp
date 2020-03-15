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

#ifndef LIBBITCOIN_MVCC_RAW_BLOCK_HPP
#define LIBBITCOIN_MVCC_RAW_BLOCK_HPP

#include <bitcoin/system.hpp>

namespace libbitcoin {
namespace database {
namespace storage {

const auto BLOCK_SIZE = 1 << 20;

// A lot of this comes from cmu db's TBD terrier project
// https://github.com/cmu-db/terrier

class alignas(BLOCK_SIZE) raw_block
{
private:
  /**
   * The insert head tells us where the next insertion will take
   * place.
   *
   * This counter is never decreased as slot recycling does not happen
   * on the fly with insertions. A background compaction process scans
   * through blocks and free up slots.

   * Since the block size is less then (1<<20) the uppper 12 bits of
   * insert_head_ is free. We use the first bit (1<<31) to indicate if
   * the block is insertable.  If the first bit is 0, the block is
   * insertable, otherwise one txn is inserting to this block
   */
  std::atomic<uint32_t> insert_head_;

public:
  /**
   * Contents of the raw block, a byte array.
   */
  system::byte_array<BLOCK_SIZE - sizeof(uint32_t)> content_;

  /**
   * Get the offset of this block. Because the first bit insert_head_ is used to indicate the status
   * of the block, we need to clear the status bit to get the real offset
   * @return the offset which tells us where the next insertion should take place
   */
  uint32_t get_insert_head() { return INT32_MAX & insert_head_.load(); }

};

/**
 * Allocator for allocating raw blocks
 */
class block_allocator {
public:
    /**
     * Allocates a new object by calling its constructor.
     * @return a pointer to the allocated object.
     */
    raw_block* allocate()
    {
        return new raw_block();
    }

    /**
     * reuse a reused chunk of memory to be handed out again
     * @param reused memory location, possibly filled with junk bytes
     */
    void reuse(raw_block* const reused)
    {
        /* no operation required */
    }

    /**
     * deallocate the object by calling its destructor.
     * @param ptr a pointer to the object to be deleted.
     */
    void deallocate(raw_block *const ptr)
    {
        delete ptr;
    }
};

} // namespace storage
} // namespace database
} // namespace libbitcoin

#endif
