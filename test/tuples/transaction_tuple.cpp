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
#include <bitcoin/database/tuples/transaction_tuple.hpp>
#include <bitcoin/database/tuples/transaction_tuple_delta.hpp>

using namespace bc::database::tuples;

BOOST_AUTO_TEST_SUITE(transaction_tuples_tests)

BOOST_AUTO_TEST_CASE(transaction_tuple__sizeof__returns_20__success)
{
    auto result = sizeof(transaction_tuple);
    BOOST_REQUIRE_EQUAL(result, 20);

    auto delta_result = sizeof(transaction_tuple_delta);
    BOOST_REQUIRE_EQUAL(delta_result, 8);
}

BOOST_AUTO_TEST_SUITE_END()
