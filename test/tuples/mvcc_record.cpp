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
#include <bitcoin/database/transaction_management/transaction_manager.hpp>
#include <bitcoin/database/tuples/block_tuple.hpp>
#include <bitcoin/database/tuples/block_tuple_delta.hpp>
#include <bitcoin/database/tuples/mvcc_record.hpp>

using namespace bc;
using namespace bc::database;
using namespace bc::database::tuples;

// block tuple wrapped in mvcc record
using block_mvcc_record = mvcc_record<block_tuple_ptr, block_delta_ptr>;

BOOST_AUTO_TEST_SUITE(mvcc_record_tests)

BOOST_AUTO_TEST_CASE(mvcc_record__usage__example__success)
{
    // start transaction 1
    transaction_manager manager;
    auto context = manager.begin_transaction();

    // create a block mvcc record using the transaction
    block_mvcc_record record(context);
    BOOST_CHECK_EQUAL(record.get_txn_id(), not_locked);
    BOOST_CHECK_EQUAL(record.get_next(), nullptr);

    // commit the transaction
    // test - record mvcc columns, data field and next field

    // start transaction 2
    // read the mvcc record
    // test - record mvcc columns, data field and next field reflect
    // read is in progress
    // commit the transaction
    // test - record mvcc columns, data field and next field reflect
    // read is no longer in progress

    // start transaction 3
    // fetch mvcc record - would be obtained from index
    // update mvcc record
    // commit the transaction
}

BOOST_AUTO_TEST_SUITE_END()
