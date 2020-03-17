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

#ifndef LIBBITCOIN_MVCC_STORAGE_UTIL_HPP
#define LIBBITCOIN_MVCC_STORAGE_UTIL_HPP

#include <bitcoin/database/define.hpp>
#include <cstddef>

namespace libbitcoin {
namespace database {
namespace storage {

/**
 * Static utility class for common functions in storage
 */
class util {
public:
  /**
   * Given a pointer, pad the pointer so that the pointer aligns to the given
   * size.
   * @param size the size to pad up to
   * @param ptr the pointer to pad
   * @return padded pointer
   */
  static uint8_t *aligned_ptr(const uint8_t, const void *);

  util() = delete;

  /**
   * Given an address offset, aligns it to the word_size
   * @param word_size size in bytes to align offset to
   * @param offset address to be aligned
   * @return modified version of address padded to align to word_size
   */
  uint32_t pad_upto_size(const uint8_t, const uint32_t);
};

} // namespace storage
} // namespace database
} // namespace libbitcoin

#endif
