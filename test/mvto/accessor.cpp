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

    BOOST_TEST_CHECKPOINT("1");
    const block_pool_ptr block_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto block_store_ptr = std::make_shared<store<block_mvcc_record>>(block_store_pool);

    BOOST_TEST_CHECKPOINT("2");
    // delta storage
    const block_pool_ptr delta_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto delta_store_ptr = std::make_shared<store<block_delta_mvcc_record>>(delta_store_pool);
    BOOST_TEST_CHECKPOINT("3");

    BOOST_TEST_CHECKPOINT("calling constructor");
    block_mvto_accessor instance{block_store_ptr, delta_store_ptr};
    BOOST_TEST_CHECKPOINT("constructor returned");
}

// heard is the first record in the O2N version chain
BOOST_AUTO_TEST_CASE(accessor__put__head_record__success)
{
    BOOST_TEST_CHECKPOINT("starting put head record test");
    const uint64_t size_limit = 1;
    const uint64_t reuse_limit = 1;

    BOOST_TEST_CHECKPOINT("4");
    const block_pool_ptr block_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto block_store_ptr = std::make_shared<store<block_mvcc_record>>(block_store_pool);

    BOOST_TEST_CHECKPOINT("5");

    // delta storage
    const block_pool_ptr delta_store_pool = std::make_shared<block_pool>(size_limit, reuse_limit);
    auto delta_store_ptr = std::make_shared<store<block_delta_mvcc_record>>(delta_store_pool);

    BOOST_TEST_CHECKPOINT("6");
    block_mvto_accessor instance{block_store_ptr, delta_store_ptr};

    transaction_manager manager;
    auto context = manager.begin_transaction();
    BOOST_TEST_CHECKPOINT("7");

    auto record_data = std::make_shared<block_tuple>();
    record_data->height = 0;

    BOOST_TEST_CHECKPOINT("calling put");
    auto slot = instance.put(context, record_data);
    BOOST_TEST_CHECKPOINT("put returned");
    //BOOST_CHECK(!slot.is_uninitialized());
}

BOOST_AUTO_TEST_SUITE_END()
