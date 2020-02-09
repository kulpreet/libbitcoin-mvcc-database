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

// // block delta record wrapped in mvcc record
// using delta_mvcc_record =
//     mvcc_record<block_delta_ptr, block_delta_ptr>;

// block tuple wrapped in mvcc record
typedef mvcc_record<block_tuple_ptr, block_delta_ptr,
                    block_reader, block_writer> block_mvcc_record;

BOOST_AUTO_TEST_SUITE(mvcc_record_tests)

BOOST_AUTO_TEST_CASE(mvcc_record__get_latch__release_latch__success)
{
    // start transaction 1
    transaction_manager manager;
    auto context = manager.begin_transaction();
    auto context2 = manager.begin_transaction();

    // create a block mvcc record using the transaction
    block_mvcc_record record(context);
    //BOOST_CHECK(record.begin() == record.end());
    BOOST_CHECK(record.get_latch_for_write(context));
    BOOST_CHECK(!record.get_latch_for_write(context2));
    BOOST_CHECK(record.release_latch(context));
}

BOOST_AUTO_TEST_CASE(mvcc_record__install__latched_by_constructor__success)
{
    // start transaction 1
    transaction_manager manager;
    auto context = manager.begin_transaction();

    // create a block mvcc record using the transaction
    block_mvcc_record record(context);
    BOOST_CHECK(record.install(context));

    // now can be lached again
    auto context2 = manager.begin_transaction();
    BOOST_CHECK(record.get_latch_for_write(context2));
}

BOOST_AUTO_TEST_CASE(mvcc_record__install__not_latched__failure)
{
    // start transaction 1
    transaction_manager manager;
    auto context = manager.begin_transaction();

    // create a block mvcc record using the transaction
    block_mvcc_record record(context);
    record.release_latch(context);
    BOOST_CHECK(!record.install(context));
}

BOOST_AUTO_TEST_CASE(mvcc_record__install__latched_by_different_context__failure)
{
    // start transaction 1
    transaction_manager manager;
    auto context = manager.begin_transaction();
    auto context2 = manager.begin_transaction();

    // create a block mvcc record using the transaction
    block_mvcc_record record(context);
    BOOST_CHECK(!record.install(context2));
}

BOOST_AUTO_TEST_CASE(mvcc_record__create_and_install_new_version_single__success)
{
    // start transaction
    transaction_manager manager;
    auto context = manager.begin_transaction();

    // create a block mvcc record using the transaction
    block_mvcc_record record(context);
    // install it
    BOOST_REQUIRE(record.install(context));

    // create first delta record
    block_delta_ptr data = std::make_shared<block_tuple_delta>();
    data->state = 1;
    block_mvcc_record::delta_mvcc_record_ptr delta = record.allocate_next(data, context);

    // set end ts, and release latch
    BOOST_REQUIRE(delta->install(context));
    // set end ts, and release latch
    BOOST_REQUIRE(record.install(context));
}

// BOOST_AUTO_TEST_CASE(mvcc_record__read_versions__single__success)
// {
//     // start transaction 1
//     transaction_manager manager;
//     auto context = manager.begin_transaction();

//     // create a block mvcc record using the transaction
//     block_mvcc_record record(context);
//     BOOST_REQUIRE(!record.install(context));

//     block_delta_ptr delta_data = std::make_shared<block_tuple_delta>();
//     delta_data->state = 1;
//     auto context2 = manager.begin_transaction();
//     delta_mvcc_record delta(delta_data, context2);
// }

    // test iterating over versions chain
    // HERE

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

BOOST_AUTO_TEST_SUITE_END()
