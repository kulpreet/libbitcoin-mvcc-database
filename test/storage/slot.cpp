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
#include <boost/test/unit_test.hpp>
#include <bitcoin/database/storage/raw_block.hpp>
#include <bitcoin/database/storage/slot.hpp>

using namespace bc;
using namespace bc::database;
using namespace bc::database::storage;

BOOST_AUTO_TEST_SUITE(slot_tests)

BOOST_AUTO_TEST_CASE(slot__access__get_correct_block__success)
{
    // get a new block
    block_allocator alloc{};
    raw_block* block = alloc.allocate();

    slot first{block, 0};
    for(uint32_t i = 1; i < 10; ++i)
    {
        slot second{block, i};
        BOOST_CHECK_EQUAL(first.get_block(), second.get_block());
        BOOST_CHECK_EQUAL(second.get_offset() - first.get_offset(), i);
    }
}

BOOST_AUTO_TEST_SUITE_END()
