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

BOOST_AUTO_TEST_SUITE(mvcc_record_tests)

BOOST_AUTO_TEST_CASE(mvcc_record__sizeof__head_and_delta__success)
{
    BOOST_CHECK_EQUAL(sizeof(block_mvcc_record), 144);

    BOOST_CHECK_EQUAL(sizeof(block_delta_mvcc_record), 48);
}

BOOST_AUTO_TEST_CASE(mvcc_record__get_latch__release_latch__success)
{
    // start transaction 1
    transaction_manager manager;
    auto context = manager.begin_transaction();
    auto context2 = manager.begin_transaction();

    // create a block mvcc record using the transaction
    block_mvcc_record record(context);
    BOOST_CHECK(record.begin() == record.end());
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
    record.install(context);
    BOOST_CHECK_EQUAL(record.get_end_timestamp(), context.get_timestamp());
    record.release_latch(context);

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
    record.install(context);
    BOOST_CHECK_EQUAL(record.get_end_timestamp(), infinity);
}

BOOST_AUTO_TEST_CASE(mvcc_record__install__latched_by_different_context__failure)
{
    // start transaction 1
    transaction_manager manager;
    auto context = manager.begin_transaction();
    auto context2 = manager.begin_transaction();

    // create a block mvcc record using the transaction
    block_mvcc_record record(context);
    record.install(context2);
    BOOST_CHECK_EQUAL(record.get_end_timestamp(), infinity);
}

BOOST_AUTO_TEST_CASE(mvcc_record__create_and_install_new_version_single__success)
{
    // start transaction
    transaction_manager manager;
    auto context = manager.begin_transaction();

    // create a block mvcc record using the transaction
    block_mvcc_record record(context);
    // install it
    record.install(context);
    BOOST_CHECK_EQUAL(record.get_end_timestamp(), context.get_timestamp());

    // create first delta record
    block_mvcc_record::delta_mvcc_record_ptr delta = record.allocate_next(context);
    delta->get_data().state = 1;

    // set end ts, and release latch
    delta->install(context);
    // set end ts
    record.install(context);
    BOOST_CHECK_EQUAL(record.get_end_timestamp(), context.get_timestamp());
}

BOOST_AUTO_TEST_CASE(mvcc_record__read__visible__success)
{
    // start transaction
    transaction_manager manager;
    auto context = manager.begin_transaction();

    // create a block mvcc record using the transaction
    auto record = std::make_shared<block_mvcc_record>(context);
    // install it
    record->install(context);
    context.register_commit_action([record, context]()
        {
            // commit to context timestamp
            record->commit(context, context.get_timestamp());
        });

    // create first delta record
    block_mvcc_record::delta_mvcc_record_ptr delta = record->allocate_next(context);
    delta->get_data().state = 1;

    BOOST_CHECK(record->get_next() == block_mvcc_record::no_next);
    // install next version and setup commit action
    record->install_next_version(delta, context);
    BOOST_CHECK(record->get_next() != block_mvcc_record::no_next);

    context.register_commit_action([delta, context]()
        {
            // commit to infinity
            delta->commit(context);
        });

    // Commit transaction context, so all the records and deltas can
    // be committed
    context.commit();

    // test record timestamps
    BOOST_CHECK(record->get_begin_timestamp() == context.get_timestamp());
    BOOST_CHECK(record->get_end_timestamp() == context.get_timestamp());
    BOOST_CHECK(record->get_read_timestamp() == none_read);

    // test delta timestamps
    BOOST_CHECK(delta->get_begin_timestamp() == context.get_timestamp());
    BOOST_CHECK(delta->get_end_timestamp() == infinity);
    BOOST_CHECK(delta->get_read_timestamp() == none_read);

    // Read the record in a new transaction
    auto context2 = manager.begin_transaction();
    auto result = record->read_record(context2, block_tuple::read_from_delta);

    BOOST_CHECK_EQUAL(record->get_read_timestamp(), context2.get_timestamp());
    BOOST_CHECK_EQUAL(delta->get_read_timestamp(), context2.get_timestamp());

    BOOST_CHECK_EQUAL(result->state, 1);
    BOOST_CHECK_EQUAL(delta->get_read_timestamp(), context2.get_timestamp());
}

// TODO: A test to capture a transaction with id less than a record's
// read_timestamp should not be able to commit

// TODO: A test to capture a transaction with id less than a delta's
// read_timestamp should not be able to commit

BOOST_AUTO_TEST_SUITE_END()
