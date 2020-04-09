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

#include <bitcoin/system.hpp>
#include <bitcoin/database/tuples/block_tuple.hpp>
#include <bitcoin/database/databases/block_database.hpp>

using namespace bc;
using namespace bc::system::chain;
using namespace bc::database;
using namespace bc::database::tuples;

transaction random_tx(size_t fudge)
{
    static const auto settings = system::settings(
        system::config::settings::mainnet);
    static const chain::block genesis = settings.genesis_block;
    auto tx = genesis.transactions()[0];
    tx.inputs()[0].previous_output().set_index(fudge);
    tx.metadata.link = fudge;
    return tx;
}

BOOST_AUTO_TEST_SUITE(block_database_tests)

BOOST_AUTO_TEST_CASE(block_database__constructor__smoke_test__success)
{
    block_database instance{10, 1, 10, 1};
    size_t height;
    BOOST_CHECK_EQUAL(instance.top(height, true), 0);
    BOOST_CHECK_EQUAL(instance.top(height, false), 0);
}

// BOOST_AUTO_TEST_CASE(block_database__store__allocate_store_and_index__success)
// {
//     block_tuple_memory_store memory_store;
//     block_database instance(memory_store);

//     static const auto settings = system::settings(system::config::settings::mainnet);
//     chain::block block0 = settings.genesis_block;
//     block0.set_transactions(
//     {
//        random_tx(0),
//        random_tx(1)
//     });

//     auto from_store = instance.store(block0.header(), 100, 1, 100, 0);
//     BOOST_REQUIRE(from_store->previous_block_hash == block0.header().previous_block_hash());
//     BOOST_REQUIRE(from_store->height == 100);

//     auto from_index = instance.get(block0.header().hash());
//     BOOST_REQUIRE_EQUAL(from_index, from_store);
// }

BOOST_AUTO_TEST_SUITE_END()
