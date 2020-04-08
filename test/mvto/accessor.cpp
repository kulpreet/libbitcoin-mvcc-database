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

#include <bitcoin/database/mvto/accessor.hpp>
#include <bitcoin/database/transaction_management/transaction_context.hpp>
#include <bitcoin/database/transaction_management/transaction_manager.hpp>
#include <bitcoin/database/tuples/block_tuple.hpp>
#include <bitcoin/database/tuples/block_tuple_delta.hpp>
#include <bitcoin/database/tuples/mvcc_record.hpp>

using namespace bc;
using namespace bc::database;
using namespace bc::database::storage;
using namespace bc::database::tuples;
using namespace bc::database::mvto;

typedef accessor<block_mvcc_record, block_delta_mvcc_record> block_mvto_accessor;

BOOST_AUTO_TEST_SUITE(accessor_tests)

BOOST_AUTO_TEST_CASE(accessor__constructor____success)
{
    const uint64_t size_limit = 1;
    const uint64_t reuse_limit = 1;

    const block_pool_ptr block_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto block_store_ptr = std::make_shared<store<block_mvcc_record>>(block_store_pool);

    // delta storage
    const block_pool_ptr delta_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto delta_store_ptr = std::make_shared<store<block_delta_mvcc_record>>(delta_store_pool);

    block_mvto_accessor instance{block_store_ptr, delta_store_ptr};
}

BOOST_AUTO_TEST_CASE(accessor__put_then_get__head_record__success)
{
    const uint64_t size_limit = 1;
    const uint64_t reuse_limit = 1;

    const block_pool_ptr block_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto block_store_ptr = std::make_shared<store<block_mvcc_record>>(block_store_pool);


    // delta storage
    const block_pool_ptr delta_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto delta_store_ptr = std::make_shared<store<block_delta_mvcc_record>>(delta_store_pool);

    block_mvto_accessor instance{block_store_ptr, delta_store_ptr};

    transaction_manager manager;
    auto context = manager.begin_transaction();

    auto record_data = std::make_shared<block_tuple>();
    record_data->height = 1010;

    auto result = instance.put(context, record_data);
    BOOST_CHECK(result);

    auto read_result = instance.get(context, result, block_tuple::read_from_delta);
    BOOST_CHECK_EQUAL(read_result->height, 1010);
}

BOOST_AUTO_TEST_CASE(accessor__get__after_update_without_commit__success)
{
    const uint64_t size_limit = 1;
    const uint64_t reuse_limit = 1;

    const block_pool_ptr block_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto block_store_ptr = std::make_shared<store<block_mvcc_record>>(block_store_pool);


    // delta storage
    const block_pool_ptr delta_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto delta_store_ptr = std::make_shared<store<block_delta_mvcc_record>>(delta_store_pool);

    block_mvto_accessor instance{block_store_ptr, delta_store_ptr};

    transaction_manager manager;
    auto context = manager.begin_transaction();

    /// Insert new record
    auto record_data = std::make_shared<block_tuple>();
    record_data->height = 1010;
    record_data->state = 5;

    auto result = instance.put(context, record_data);
    BOOST_REQUIRE(result);

    /// Update record with delta
    auto delta_data = std::make_shared<block_tuple_delta>();
    delta_data->state = 10;

    BOOST_REQUIRE(instance.update(context, result, delta_data));

    auto read_result = instance.get(context, result, block_tuple::read_from_delta);
    BOOST_CHECK_EQUAL(read_result->height, 1010);
    BOOST_CHECK_EQUAL(read_result->state, 10);
}

// Read in future context (post commit) should succeed
BOOST_AUTO_TEST_CASE(accessor__get__after_update_with_commit__success)
{
    const uint64_t size_limit = 1;
    const uint64_t reuse_limit = 1;

    const block_pool_ptr block_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto block_store_ptr = std::make_shared<store<block_mvcc_record>>(block_store_pool);


    // delta storage
    const block_pool_ptr delta_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto delta_store_ptr = std::make_shared<store<block_delta_mvcc_record>>(delta_store_pool);

    block_mvto_accessor instance{block_store_ptr, delta_store_ptr};

    transaction_manager manager;
    auto context = manager.begin_transaction();

    /// Insert new record
    auto record_data = std::make_shared<block_tuple>();
    record_data->height = 1010;
    record_data->state = 5;

    auto result = instance.put(context, record_data);
    BOOST_REQUIRE(result);

    context.commit();

    auto context2 = manager.begin_transaction();

    /// Update record with delta
    auto delta_data = std::make_shared<block_tuple_delta>();
    delta_data->state = 10;

    BOOST_REQUIRE(instance.update(context, result, delta_data));

    context2.commit();

    auto context3 = manager.begin_transaction();

    auto read_result = instance.get(context, result, block_tuple::read_from_delta);
    BOOST_CHECK_EQUAL(read_result->height, 1010);
    BOOST_CHECK_EQUAL(read_result->state, 10);
}

// Read in a future context (pre commit) should fail?
BOOST_AUTO_TEST_CASE(accessor__get__read_before_update_commit__get_old_value)
{
    const uint64_t size_limit = 1;
    const uint64_t reuse_limit = 1;

    const block_pool_ptr block_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto block_store_ptr = std::make_shared<store<block_mvcc_record>>(block_store_pool);

    // delta storage
    const block_pool_ptr delta_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto delta_store_ptr = std::make_shared<store<block_delta_mvcc_record>>(delta_store_pool);

    block_mvto_accessor instance{block_store_ptr, delta_store_ptr};

    transaction_manager manager;
    auto context = manager.begin_transaction();

    /// Insert new record
    auto record_data = std::make_shared<block_tuple>();
    record_data->height = 1010;
    record_data->state = 5;

    auto result = instance.put(context, record_data);
    BOOST_REQUIRE(result);

    context.commit();

    auto context2 = manager.begin_transaction();
    auto context3 = manager.begin_transaction();

    auto read_result = instance.get(context2, result, block_tuple::read_from_delta);
    BOOST_CHECK(read_result != block_mvcc_record::not_found);

    /// Update record with delta
    auto delta_data = std::make_shared<block_tuple_delta>();
    delta_data->state = 10;

    BOOST_REQUIRE(instance.update(context2, result, delta_data));

    // head is still write latched by context2, so not_found
    read_result = instance.get(context3, result, block_tuple::read_from_delta);
    BOOST_CHECK_EQUAL(read_result, block_mvcc_record::not_found);

    // write latch on head should result in update failure
    BOOST_REQUIRE(!instance.update(context3, result, delta_data));
}

// Read in past context should fail
BOOST_AUTO_TEST_CASE(accessor__get__in_old_context_after_update_with_commit__success)
{
    const uint64_t size_limit = 1;
    const uint64_t reuse_limit = 1;

    const block_pool_ptr block_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto block_store_ptr = std::make_shared<store<block_mvcc_record>>(block_store_pool);


    // delta storage
    const block_pool_ptr delta_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto delta_store_ptr = std::make_shared<store<block_delta_mvcc_record>>(delta_store_pool);

    block_mvto_accessor instance{block_store_ptr, delta_store_ptr};

    transaction_manager manager;
    auto context = manager.begin_transaction();
    auto context2 = manager.begin_transaction();

    /// Insert new record
    auto record_data = std::make_shared<block_tuple>();
    record_data->height = 1010;
    record_data->state = 5;

    auto result = instance.put(context2, record_data);
    BOOST_REQUIRE(result);

    /// Update record with delta
    auto delta_data = std::make_shared<block_tuple_delta>();
    delta_data->state = 10;

    BOOST_REQUIRE(instance.update(context2, result, delta_data));

    context2.commit();

    auto read_result = instance.get(context, result, block_tuple::read_from_delta);
    BOOST_CHECK_EQUAL(read_result, block_mvcc_record::not_found);
}

// TODO: Create version chain with three deltas - read third version
BOOST_AUTO_TEST_CASE(accessor__get__third_version__success)
{
    const uint64_t size_limit = 10;
    const uint64_t reuse_limit = 1;

    const block_pool_ptr block_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto block_store_ptr = std::make_shared<store<block_mvcc_record>>(block_store_pool);

    // delta storage
    const block_pool_ptr delta_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto delta_store_ptr = std::make_shared<store<block_delta_mvcc_record>>(delta_store_pool);

    block_mvto_accessor instance{block_store_ptr, delta_store_ptr};

    transaction_manager manager;
    auto context = manager.begin_transaction();

    /// Insert new record
    auto record_data = std::make_shared<block_tuple>();
    record_data->height = 1010;
    record_data->state = 0;

    auto result = instance.put(context, record_data);
    BOOST_REQUIRE(result);
    context.commit();

    /// Update record with delta
    auto context2 = manager.begin_transaction();
    auto delta_data = std::make_shared<block_tuple_delta>();
    delta_data->state = 1;
    BOOST_REQUIRE(instance.update(context2, result, delta_data));
    context2.commit();

    auto context3 = manager.begin_transaction();
    auto delta_data2 = std::make_shared<block_tuple_delta>();
    delta_data2->state = 2;
    BOOST_REQUIRE(instance.update(context3, result, delta_data2));
    context3.commit();

    auto context4 = manager.begin_transaction();
    auto read_result = instance.get(context4, result, block_tuple::read_from_delta);
    BOOST_CHECK(read_result != block_mvcc_record::not_found);
    BOOST_CHECK_EQUAL(read_result->height, 1010);
    BOOST_CHECK_EQUAL(read_result->state, 2);

    // latch last version for update
    delta_data = std::make_shared<block_tuple_delta>();
    delta_data->state = 3;
    BOOST_REQUIRE(instance.update(context4, result, delta_data));

    // should read the last but one version
    auto context5 = manager.begin_transaction();
    read_result = instance.get(context5, result, block_tuple::read_from_delta);
    BOOST_CHECK(read_result != block_mvcc_record::not_found);
    BOOST_CHECK_EQUAL(read_result->height, 1010);
    BOOST_CHECK_EQUAL(read_result->state, 1);
}

BOOST_AUTO_TEST_SUITE_END()
