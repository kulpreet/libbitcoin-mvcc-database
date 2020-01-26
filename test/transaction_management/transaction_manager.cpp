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

using namespace bc;
using namespace bc::database;

BOOST_AUTO_TEST_SUITE(transaction_manager_tests)

BOOST_AUTO_TEST_CASE(transaction_manager__lifecycle__start_commit__success)
{
    transaction_manager manager;
    auto context = manager.begin_transaction();
    BOOST_TEST(manager.is_active(context));

    auto res = context.get_state() == state::active;
    BOOST_CHECK(res);

    BOOST_CHECK_EQUAL(context.get_timestamp(), 1);

    manager.commit_transaction(context);
    BOOST_CHECK(!manager.is_active(context));

    res = context.get_state() == state::committed;
    BOOST_CHECK(res);

    manager.remove_transaction(context);
    BOOST_CHECK(!manager.is_active(context));
}

BOOST_AUTO_TEST_CASE(transaction_manager__remove_transaction__active__fails)
{
    transaction_manager manager;
    auto context = manager.begin_transaction();
    BOOST_CHECK(manager.is_active(context));

    manager.remove_transaction(context);
    BOOST_CHECK(!manager.is_active(context));
}

BOOST_AUTO_TEST_SUITE_END()
