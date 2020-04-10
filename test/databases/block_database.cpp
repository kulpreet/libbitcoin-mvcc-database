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
#include <bitcoin/database/block_state.hpp>
#include <bitcoin/database/transaction_management/transaction_manager.hpp>
#include <bitcoin/database/transaction_management/transaction_context.hpp>
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
    size_t height = -1;

    transaction_manager manager;
    auto context = manager.begin_transaction();
    BOOST_CHECK(!instance.top(context, height, true));
    BOOST_CHECK_EQUAL(height, -1);

    height = -1;
    context = manager.begin_transaction();
    BOOST_CHECK(!instance.top(context, height, false));
    BOOST_CHECK_EQUAL(height, -1);
}

BOOST_AUTO_TEST_CASE(block_database__store__allocate_store_and_index__success)
{
    static const auto settings = system::settings(system::config::settings::mainnet);
    chain::block block0 = settings.genesis_block;
    block0.set_transactions(
    {
       random_tx(0),
       random_tx(1)
    });

    transaction_manager manager;
    auto context = manager.begin_transaction();

    block_database instance{10, 1, 10, 1};

    BOOST_REQUIRE(instance.store(context, block0.header(), 100, 1, 200, block_state::candidate));

    auto from_index = instance.get(context, block0.header().hash());
    BOOST_CHECK(from_index->merkle_root == block0.header().merkle_root());

    // block is not promoted yet, so not in candidate or confirmed indexes
    from_index = instance.get(context, 0, true);
    BOOST_CHECK(from_index == nullptr);

    // start new context, as last get aborted the transaction
    context = manager.begin_transaction();
    size_t height = -1;
    BOOST_CHECK(!instance.top(context, height, true));
    BOOST_CHECK_EQUAL(height, -1);

    height = -1;
    BOOST_CHECK(!instance.top(context, height, true));
    BOOST_CHECK_EQUAL(height, -1);
}

BOOST_AUTO_TEST_CASE(block_database__promote__candidate_then_confirmed__success)
{
    static const auto settings = system::settings(system::config::settings::mainnet);
    chain::block block0 = settings.genesis_block;
    block0.set_transactions(
    {
       random_tx(0),
       random_tx(1)
    });

    transaction_manager manager;
    auto context = manager.begin_transaction();

    block_database instance{10, 1, 10, 1};

    BOOST_REQUIRE(instance.store(context, block0.header(), 100, 1, 200, block_state::candidate));

    // Candidate
    BOOST_REQUIRE(instance.promote(context, block0.header().hash(), 0, true));

    size_t height = -1;
    BOOST_CHECK(instance.top(context, height, true));
    BOOST_CHECK_EQUAL(height, 0);

    height = -1;
    BOOST_CHECK(!instance.top(context, height, false));
    BOOST_CHECK_EQUAL(height, -1);

    auto reloaded = instance.get(context, block0.header().hash());
    BOOST_CHECK_EQUAL(reloaded->state, block_state::candidate);

    // Confirm
    BOOST_REQUIRE(instance.promote(context, block0.header().hash(), 0, false));

    height = -1;
    BOOST_CHECK(instance.top(context, height, true));
    BOOST_CHECK_EQUAL(height, 0);

    height = -1;
    BOOST_CHECK(instance.top(context, height, false));
    BOOST_CHECK_EQUAL(height, 0);

    reloaded = instance.get(context, block0.header().hash());
    BOOST_CHECK_EQUAL(reloaded->state, block_state::confirmed);
}

BOOST_AUTO_TEST_CASE(block_database__get_header_metadata__smoke_test__success)
{
    static const auto settings = system::settings(system::config::settings::mainnet);
    chain::block block0 = settings.genesis_block;
    block0.set_transactions(
    {
       random_tx(0),
       random_tx(1)
    });

    transaction_manager manager;
    auto context = manager.begin_transaction();

    block_database instance{10, 1, 10, 1};

    BOOST_REQUIRE(instance.store(context, block0.header(), 100, 1, 200, block_state::candidate));
    instance.get_header_metadata(context, block0.header());

    BOOST_REQUIRE_EQUAL(block0.header().metadata.error, error::success);
    BOOST_REQUIRE(block0.header().metadata.exists);
    // BOOST_REQUIRE(block0.header().metadata.populated);
    BOOST_REQUIRE(!block0.header().metadata.validated);
    BOOST_REQUIRE(block0.header().metadata.candidate);
    BOOST_REQUIRE(!block0.header().metadata.confirmed);
}

BOOST_AUTO_TEST_CASE(block_database__validate__from_candidate__success)
{
    static const auto settings = system::settings(system::config::settings::mainnet);
    chain::block block0 = settings.genesis_block;
    block0.set_transactions(
    {
       random_tx(0),
       random_tx(1)
    });

    transaction_manager manager;
    auto context = manager.begin_transaction();

    block_database instance{10, 1, 10, 1};

    BOOST_REQUIRE(instance.store(context, block0.header(), 100, 1, 200, block_state::candidate));

    // Validate
    BOOST_REQUIRE(instance.validate(context, block0.header().hash(), error::success));

    auto reloaded = instance.get(context, block0.header().hash());
    BOOST_CHECK_EQUAL(reloaded->state, block_state::valid | block_state::candidate);
}

BOOST_AUTO_TEST_SUITE_END()
