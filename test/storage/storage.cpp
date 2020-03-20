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

#include <bitcoin/database/storage/storage.hpp>
#include <bitcoin/database/transaction_management/transaction_manager.hpp>
#include <bitcoin/database/tuples/block_tuple.hpp>
#include <bitcoin/database/tuples/block_tuple_delta.hpp>
#include <bitcoin/database/tuples/mvcc_record.hpp>

using namespace bc;
using namespace bc::database;
using namespace bc::database::storage;
using namespace bc::database::tuples;

BOOST_AUTO_TEST_SUITE(storage_tests)

BOOST_AUTO_TEST_CASE(storage__constructor____success)
{
  const uint64_t size_limit = 1;
  const uint64_t reuse_limit = 1;
  const block_pool_ptr block_store = std::make_shared<block_pool>(size_limit, reuse_limit);

  store<block_mvcc_record> instance{block_store};
}

BOOST_AUTO_TEST_CASE(storage__insert_update_read__block_mvcc_record__success)
{
  const uint64_t size_limit = 1;
  const uint64_t reuse_limit = 1;
  const block_pool_ptr block_store = std::make_shared<block_pool>(size_limit, reuse_limit);

  store<block_mvcc_record> instance{block_store};

  transaction_manager manager;
  auto context = manager.begin_transaction();
  block_mvcc_record record(context);
  auto record_slot = instance.insert(context, record);

  // delta storage
  const block_pool_ptr delta_store = std::make_shared<block_pool>(size_limit, reuse_limit);
  store<block_delta_mvcc_record> instance2{delta_store};

  // use constructor, will give us a latched record
  block_delta_mvcc_record delta_record(context);
  auto delta_slot = instance2.insert(context, delta_record);

  // We need to install the records next, and set the commit actions
  auto record_bytes = record_slot.get_bytes();
  auto record_ptr = reinterpret_cast<block_mvcc_record*>(record_bytes);
  record_ptr->install(context);
  context.register_end_action([record_ptr, context]()
  {
      // commit to context timestamp
      record_ptr->commit(context, context.get_timestamp());
  });

  BOOST_CHECK(record_ptr->get_next() == block_mvcc_record::no_next);

  auto delta_bytes = delta_slot.get_bytes();
  auto delta_ptr = reinterpret_cast<block_mvcc_record::delta_mvcc_record*>(delta_bytes);

  record_ptr->install_next_version(delta_ptr, context);
  BOOST_CHECK(record_ptr->get_next() != block_mvcc_record::no_next);

  context.register_end_action([delta_ptr, context]()
  {
      // commit to infinity
      delta_ptr->commit(context);
  });

  context.register_end_action([record_ptr, context]()
  {
      // commit to context timestamp
      record_ptr->commit(context, context.get_timestamp());
  });

  // Next we commit the transaction
  context.commit();

  // test record timestamps
  BOOST_CHECK(record_ptr->get_begin_timestamp() == context.get_timestamp());
  BOOST_CHECK(record_ptr->get_end_timestamp() == context.get_timestamp());
  BOOST_CHECK(record_ptr->get_read_timestamp() == none_read);

  // test delta timestamps
  BOOST_CHECK(delta_ptr->get_begin_timestamp() == context.get_timestamp());
  BOOST_CHECK(delta_ptr->get_end_timestamp() == infinity);
  BOOST_CHECK(delta_ptr->get_read_timestamp() == none_read);

  // Then we read the records using the slot returned from first
  // insert and then read through all versions.
  auto context2 = manager.begin_transaction();
  auto read_result = instance.read(record_slot, context2, block_tuple::read_from_delta);
}

BOOST_AUTO_TEST_SUITE_END()
