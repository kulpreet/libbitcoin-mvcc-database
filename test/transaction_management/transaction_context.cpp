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

#include <bitcoin/database/transaction_management/transaction_manager.hpp>
#include <bitcoin/database/transaction_management/transaction_context.hpp>

using namespace bc;
using namespace bc::database;

BOOST_AUTO_TEST_SUITE(transaction_context_tests)

BOOST_AUTO_TEST_CASE(transaction_context__commit_actions__on_commit__success)
{
    transaction_manager manager;
    auto context = manager.begin_transaction();

    auto count = 0;
    context.register_commit_action([&count]()
    {
        count++;
    });
    context.register_commit_action([&count]()
    {
        count++;
    });
    context.commit();
    BOOST_CHECK_EQUAL(count, 2);
    BOOST_CHECK(context.get_state() == state::committed);
}

BOOST_AUTO_TEST_CASE(transaction_context__abort_actions__on_abort__success)
{
    transaction_manager manager;
    auto context = manager.begin_transaction();

    auto count = 0;
    context.register_abort_action([&count]()
    {
        count++;
    });
    context.register_abort_action([&count]()
    {
        count++;
    });
    context.abort();
    BOOST_CHECK_EQUAL(count, 2);
    BOOST_CHECK(context.get_state() == state::aborted);
}

BOOST_AUTO_TEST_CASE(transaction_context__abort_actions__on_commit__success)
{
    transaction_manager manager;
    auto context = manager.begin_transaction();

    auto count = 0;
    context.register_abort_action([&count]()
    {
        count++;
    });
    context.commit();
    BOOST_CHECK_EQUAL(count, 0);
    BOOST_CHECK(context.get_state() == state::committed);
}

BOOST_AUTO_TEST_CASE(transaction_context__commit_actions__on_abort__success)
{
    transaction_manager manager;
    auto context = manager.begin_transaction();

    auto count = 0;
    context.register_commit_action([&count]()
    {
        count++;
    });
    context.abort();
    BOOST_CHECK_EQUAL(count, 0);
    BOOST_CHECK(context.get_state() == state::aborted);
}

BOOST_AUTO_TEST_SUITE_END()
