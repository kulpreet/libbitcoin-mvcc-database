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

#include <cstddef>
#include <bitcoin/database/storage/util.hpp>

namespace libbitcoin {
namespace database {
namespace storage {

uint32_t util::pad_upto_size(const uint8_t word_size, const uint32_t offset) {
  BITCOIN_ASSERT_MSG((word_size & (word_size - 1)) == 0,
      "word_size should be a power of two.");
  // Because size is a power of two, mask is always all 1s up to the length of size.
  // example, size is 8 (1000), mask is (0111)
  uint32_t mask = word_size - 1;
  // This is equivalent to (offset + (size - 1)) / size, which always pads up as desired
  return (offset + mask) & (~mask);
}

uint8_t *aligned_ptr(const uint8_t size, const void *ptr)
{
    BITCOIN_ASSERT_MSG((size & (size - 1)) == 0,
        "word_size should be a power of two.");
    // Because size is a power of two, mask is always all 1s up to the length of size.
    // example, size is 8 (1000), mask is (0111)
    uintptr_t mask = size - 1;
    auto ptr_value = reinterpret_cast<uintptr_t>(ptr);
    // This is equivalent to (value + (size - 1)) / size.
    return reinterpret_cast<uint8_t *>((ptr_value + mask) & (~mask));
}


} // namespace storage
} // namespace database
} // namespace libbitcoin
