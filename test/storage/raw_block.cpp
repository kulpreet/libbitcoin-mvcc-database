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

using namespace bc;
using namespace bc::database;
using namespace bc::database::storage;

BOOST_AUTO_TEST_SUITE(raw_block_tests)

BOOST_AUTO_TEST_CASE(raw_block__set_busy_status__not_set__success)
{
    // get a new block
    block_allocator alloc{};
    raw_block* block = alloc.allocate();
    // set busy status, should succeed
    BOOST_CHECK(block->set_busy_status());
}

BOOST_AUTO_TEST_CASE(raw_block__set_busy_status__already_set__failure)
{
    // get a new block
    block_allocator alloc{};
    raw_block* block = alloc.allocate();
    // set busy status
    BOOST_CHECK(block->set_busy_status());

    // set busy status should fail
    BOOST_CHECK(!block->set_busy_status());
}

BOOST_AUTO_TEST_CASE(raw_block__clear_busy_status__already_set__success)
{
    // get a new block
    block_allocator alloc{};
    raw_block* block = alloc.allocate();
    // set busy status
    BOOST_CHECK(block->set_busy_status());

    // clear busy status should succeed
    BOOST_CHECK(block->clear_busy_status());
}

BOOST_AUTO_TEST_CASE(raw_block__clear_busy_status__not_set__failure)
{
    // get a new block
    block_allocator alloc{};
    raw_block* block = alloc.allocate();

    // clear busy status should fail
    BOOST_CHECK(!block->clear_busy_status());
}

BOOST_AUTO_TEST_SUITE_END()
