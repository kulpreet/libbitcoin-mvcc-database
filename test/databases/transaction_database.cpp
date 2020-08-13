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
#include <bitcoin/database/tuples/transaction_tuple.hpp>
#include <bitcoin/database/databases/transaction_database.hpp>

using namespace bc;
using namespace bc::system::chain;
using namespace bc::database;
using namespace bc::database::tuples;

BOOST_AUTO_TEST_SUITE(transaction_database_tests)

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

BOOST_AUTO_TEST_CASE(transaction_database__constructor__smoke_test__success)
{
    transaction_database instance{10, 1, 10, 1};
    size_t height = -1;

    // transaction_manager manager;
    // auto context = manager.begin_transaction();
    // BOOST_CHECK(!instance.top(context, height, true));
    // BOOST_CHECK_EQUAL(height, -1);

    // height = -1;
    // context = manager.begin_transaction();
    // BOOST_CHECK(!instance.top(context, height, false));
    // BOOST_CHECK_EQUAL(height, -1);
}

BOOST_AUTO_TEST_SUITE_END()
