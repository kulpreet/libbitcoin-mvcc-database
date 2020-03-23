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
#include <bitcoin/database/transaction_management/transaction_context.hpp>
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
  auto new_block = instance.get_current_block();
  for (uint32_t i = 0; i < BLOCK_SIZE - sizeof(uint32_t); i++) {
      BOOST_CHECK_EQUAL(new_block->content_[i], 0);
  }
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
  delta_record.get_data().state = 1;
  auto delta_slot = instance2.insert(context, delta_record);

  // We need to install the records next, and set the commit actions
  auto record_bytes = record_slot.get_bytes();
  auto record_ptr = reinterpret_cast<block_mvcc_record*>(record_bytes);
  record_ptr->install(context);
  context.register_commit_action([record_ptr, context]()
  {
      // commit to context timestamp
      record_ptr->commit(context, context.get_timestamp());
  });

  BOOST_CHECK(record_ptr->get_next() == block_mvcc_record::no_next);

  auto delta_bytes = delta_slot.get_bytes();
  auto delta_ptr = reinterpret_cast<block_mvcc_record::delta_mvcc_record*>(delta_bytes);

  record_ptr->install_next_version(delta_ptr, context);
  BOOST_CHECK(record_ptr->get_next() != block_mvcc_record::no_next);

  context.register_commit_action([delta_ptr, context]()
  {
      // commit to infinity
      delta_ptr->commit(context);
  });

  context.register_commit_action([record_ptr, context]()
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

  BOOST_CHECK_EQUAL(read_result->state, 1);
}

BOOST_AUTO_TEST_CASE(storage__insert_record__should_be_latched__success)
{
  const uint64_t size_limit = 1;
  const uint64_t reuse_limit = 1;
  const block_pool_ptr block_store = std::make_shared<block_pool>(size_limit, reuse_limit);

  store<block_mvcc_record> instance{block_store};

  transaction_manager manager;
  auto context1 = manager.begin_transaction();
  auto context = manager.begin_transaction();
  block_mvcc_record record(context);
  auto record_slot = instance.insert(context, record);

  auto record_bytes = record_slot.get_bytes();
  auto record_ptr = reinterpret_cast<block_mvcc_record*>(record_bytes);
  BOOST_REQUIRE(record_ptr->is_latched_by(context));
}

BOOST_AUTO_TEST_CASE(storage__insert_delta__should_be_latched__success)
{
  const uint64_t size_limit = 1;
  const uint64_t reuse_limit = 1;
  const block_pool_ptr delta_store = std::make_shared<block_pool>(size_limit, reuse_limit);

  store<block_delta_mvcc_record> instance{delta_store};

  transaction_manager manager;
  auto context1 = manager.begin_transaction();
  auto context = manager.begin_transaction();
  block_delta_mvcc_record delta_record(context);
  auto delta_slot = instance.insert(context, delta_record);

  auto delta_bytes = delta_slot.get_bytes();
  auto delta_ptr = reinterpret_cast<block_mvcc_record::delta_mvcc_record*>(delta_bytes);
  BOOST_REQUIRE(delta_ptr->is_latched_by(context));
}

BOOST_AUTO_TEST_CASE(storage__insert_update_read__different_contexts__success)
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

  // We need to install the records next, and set the commit actions
  auto record_bytes = record_slot.get_bytes();
  auto record_ptr = reinterpret_cast<block_mvcc_record*>(record_bytes);
  record_ptr->install(context);
  context.register_commit_action([record_ptr, context]()
  {
      // commit to context timestamp
      record_ptr->commit(context, context.get_timestamp());
  });

  BOOST_CHECK_EQUAL(record_ptr->get_next(), block_mvcc_record::no_next);

  // commit the insert transaction
  context.commit();

  // Start new transaction for update
  auto context2 = manager.begin_transaction();
  BOOST_REQUIRE_EQUAL(context2.get_timestamp(), 2);

  // use constructor, will give us a latched record
  block_delta_mvcc_record delta_record(context2);
  BOOST_REQUIRE(delta_record.is_latched_by(context2));

  delta_record.get_data().state = 1;
  auto delta_slot = instance2.insert(context2, delta_record);

  auto delta_bytes = delta_slot.get_bytes();
  auto delta_ptr = reinterpret_cast<block_mvcc_record::delta_mvcc_record*>(delta_bytes);
  BOOST_REQUIRE(delta_ptr->is_latched_by(context2));

  BOOST_REQUIRE(record_ptr->install_next_version(delta_ptr, context2));
  BOOST_CHECK_EQUAL(record_ptr->get_next(), delta_ptr);
  BOOST_CHECK(delta_ptr->get_next() != block_mvcc_record::no_next);

  context2.register_commit_action([delta_ptr, record_ptr, context2]()
  {
      // commit to infinity
      delta_ptr->commit(context2);

      // commit to context2's timestamp
      record_ptr->commit(context2, context2.get_timestamp());
  });

  // commit the update transaction
  context2.commit();

  // test record timestamps
  BOOST_CHECK_EQUAL(record_ptr->get_begin_timestamp(), context.get_timestamp());
  BOOST_CHECK_EQUAL(record_ptr->get_end_timestamp(), context2.get_timestamp());
  BOOST_CHECK_EQUAL(record_ptr->get_read_timestamp(), none_read);
  BOOST_CHECK(!record_ptr->is_latched_by(context2));

  // test delta timestamps
  BOOST_CHECK_EQUAL(delta_ptr->get_begin_timestamp(), context2.get_timestamp());
  BOOST_CHECK_EQUAL(delta_ptr->get_end_timestamp(), infinity);
  BOOST_CHECK_EQUAL(delta_ptr->get_read_timestamp(), none_read);
  BOOST_CHECK(!delta_ptr->is_latched_by(context2));

  // Then we read the records using the slot returned from first
  // insert and then read through all versions.
  auto context3 = manager.begin_transaction();
  auto read_result = instance.read(record_slot, context3, block_tuple::read_from_delta);

  BOOST_CHECK_EQUAL(read_result->state, 1);
  BOOST_CHECK_EQUAL(record_ptr->get_read_timestamp(), context3.get_timestamp());
  BOOST_CHECK_EQUAL(delta_ptr->get_read_timestamp(), context3.get_timestamp());
}

BOOST_AUTO_TEST_CASE(storage__multple_inserts_updates_reads__different_contexts__success)
{
  const uint64_t size_limit = 1;
  const uint64_t reuse_limit = 1;

  // record storage
  const block_pool_ptr block_store = std::make_shared<block_pool>(size_limit, reuse_limit);
  store<block_mvcc_record> instance{block_store};

  // delta storage
  const block_pool_ptr delta_store = std::make_shared<block_pool>(size_limit, reuse_limit);
  store<block_delta_mvcc_record> instance2{delta_store};

  std::list<transaction_context> contexts{};
  std::list<slot> record_slots{};

  transaction_manager manager;

  for(int i = 0; i < 10; i++)
  {
      auto context = manager.begin_transaction();
      block_mvcc_record record(context);
      record.get_data().height = 0;
      auto record_slot = instance.insert(context, record);

      // We need to install the records next, and set the commit actions
      auto record_bytes = record_slot.get_bytes();
      auto record_ptr = reinterpret_cast<block_mvcc_record*>(record_bytes);
      record_ptr->install(context);
      context.register_commit_action([record_ptr, context]()
      {
          // commit to context timestamp
          record_ptr->commit(context, context.get_timestamp());
      });

      // commit the insert transaction
      context.commit();
      BOOST_CHECK(record_ptr->get_next() == block_mvcc_record::no_next);

      contexts.push_back(context);
      record_slots.push_back(record_slot);

      for(int j = 0; j < 3; j++)
      {

          // Start new transaction for update
          auto context2 = manager.begin_transaction();

          // use constructor, will give us a latched record
          block_delta_mvcc_record delta_record(context2);
          delta_record.get_data().state = j;
          auto delta_slot = instance2.insert(context2, delta_record);

          auto delta_bytes = delta_slot.get_bytes();
          auto delta_ptr = reinterpret_cast<block_mvcc_record::delta_mvcc_record*>(delta_bytes);

          BOOST_REQUIRE(record_ptr->install_next_version(delta_ptr, context2));
          BOOST_CHECK(record_ptr->get_next() != block_mvcc_record::no_next);

          context2.register_commit_action([delta_ptr, record_ptr, context2]()
          {
              // commit to infinity
              delta_ptr->commit(context2);

              // commit to context
              record_ptr->commit(context2, context2.get_timestamp());
          });

          // commit the update transaction
          context2.commit();

          // test delta timestamps
          BOOST_CHECK_EQUAL(delta_ptr->get_begin_timestamp(), context2.get_timestamp());
          BOOST_CHECK_EQUAL(delta_ptr->get_end_timestamp(), infinity);
          BOOST_CHECK_EQUAL(delta_ptr->get_read_timestamp(), none_read);

          // test record timestamps
          BOOST_CHECK_EQUAL(record_ptr->get_begin_timestamp(), context.get_timestamp());
          BOOST_CHECK_EQUAL(record_ptr->get_end_timestamp(), context2.get_timestamp());
          BOOST_CHECK_EQUAL(record_ptr->get_read_timestamp(), none_read);
      }
  }

  for(std::list<slot>::iterator s = record_slots.begin(); s != record_slots.end(); s++)
  {
      // Then we read the records using the slot returned from first
      // insert and then read through all versions.
      auto read_context = manager.begin_transaction();
      auto read_result = instance.read(*s, read_context, block_tuple::read_from_delta);

      BOOST_CHECK_EQUAL(read_result->state, 2);
  }
}

BOOST_AUTO_TEST_SUITE_END()
