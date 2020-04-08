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

BOOST_AUTO_TEST_CASE(storage__insert__multiple__success)
{
  const uint64_t size_limit = 10;
  const uint64_t reuse_limit = 1;
  const block_pool_ptr pool = std::make_shared<block_pool>(size_limit, reuse_limit);

  store<block_mvcc_record> instance{pool};

  transaction_manager manager;
  auto context = manager.begin_transaction();
  block_mvcc_record record(context);
  auto record_slot = instance.insert(context, record);
  auto record_slot1 = instance.insert(context, record);
  auto record_slot2 = instance.insert(context, record);

  BOOST_CHECK_EQUAL(record_slot.get_block(), record_slot.get_block());
  BOOST_CHECK_EQUAL(record_slot1.get_block(), record_slot2.get_block());

  BOOST_CHECK_EQUAL(record_slot1.get_offset() - record_slot.get_offset(), 1);
  BOOST_CHECK_EQUAL(record_slot2.get_offset() - record_slot1.get_offset(), 1);
  BOOST_CHECK_EQUAL(record_slot2.get_offset() - record_slot.get_offset(), 2);
}

BOOST_AUTO_TEST_CASE(storage__insert__different_stores__success)
{
  const uint64_t size_limit = 10;
  const uint64_t reuse_limit = 1;
  const block_pool_ptr pool = std::make_shared<block_pool>(size_limit, reuse_limit);
  const block_pool_ptr pool1 = std::make_shared<block_pool>(size_limit, reuse_limit);

  store<block_mvcc_record> instance{pool};
  store<block_mvcc_record> instance1{pool1};

  transaction_manager manager;
  auto context = manager.begin_transaction();

  block_mvcc_record record1(context);
  record1.get_data().height = 123456;

  block_mvcc_record record2(context);
  record1.get_data().height = 789101;

  auto record_slot1 = instance.insert(context, record1);
  auto record_slot2 = instance1.insert(context, record2);

  BOOST_CHECK(record_slot1 != record_slot2);
}

BOOST_AUTO_TEST_CASE(storage__insert_multiple_times__block_mvcc_record__success)
{
  const uint64_t size_limit = 10;
  const uint64_t reuse_limit = 1;
  const block_pool_ptr block_store = std::make_shared<block_pool>(size_limit, reuse_limit);

  store<block_mvcc_record> instance{block_store};

  transaction_manager manager;
  auto context = manager.begin_transaction();
  block_mvcc_record record(context);
  auto record_slot = instance.insert(context, record);
  auto record_slot1 = instance.insert(context, record);
  auto record_slot2 = instance.insert(context, record);

  BOOST_CHECK(record_slot != record_slot1);
  BOOST_CHECK(record_slot1 != record_slot2);
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
  auto record_ptr = instance.get_bytes_at(record_slot);
  record_ptr->install(context);
  context.register_commit_action([record_ptr, context]()
  {
      // commit to context timestamp
      record_ptr->commit(context, context.get_timestamp());
  });

  BOOST_CHECK(record_ptr->get_next() == block_mvcc_record::no_next);

  auto delta_ptr = instance2.get_bytes_at(delta_slot);

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

  auto record_ptr = instance.get_bytes_at(record_slot);
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

  auto delta_ptr = instance.get_bytes_at(delta_slot);
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
  auto record_ptr = instance.get_bytes_at(record_slot);
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

  auto delta_ptr = instance2.get_bytes_at(delta_slot);
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
  BOOST_REQUIRE_EQUAL(delta_ptr->get_txn_id(), 0);

  auto context3 = manager.begin_transaction();
  block_delta_mvcc_record delta_record2(context3);
  BOOST_REQUIRE(delta_record2.is_latched_by(context3));

  delta_record2.get_data().state = 2;
  auto delta_slot2 = instance2.insert(context3, delta_record2);

  // Just to test that inserting second tuple doesn't mess up previous
  // tuple
  BOOST_REQUIRE_EQUAL(delta_ptr->get_txn_id(), 0);

  auto delta_ptr2 = instance2.get_bytes_at(delta_slot2);
  BOOST_REQUIRE(delta_ptr2->is_latched_by(context3));

  // set up second version in the chain
  BOOST_REQUIRE(delta_ptr->install_next_version(delta_ptr2, context3));
  BOOST_CHECK_EQUAL(record_ptr->get_next()->get_next(), delta_ptr2);
  BOOST_CHECK(delta_ptr2->get_next() != block_mvcc_record::no_next);

  context3.register_commit_action([delta_ptr2, delta_ptr, context3]()
  {
      // commit to infinity
      delta_ptr2->commit(context3);

      // commit to context3's timestamp
      delta_ptr->commit(context3, context3.get_timestamp());
  });

  // commit the update transaction
  context3.commit();

  // Then we read the records using the slot returned from first
  // insert and then read through all versions.
  auto context4 = manager.begin_transaction();
  auto read_result = instance.read(record_slot, context4, block_tuple::read_from_delta);

  BOOST_CHECK_EQUAL(read_result->state, 2);
  BOOST_CHECK_EQUAL(record_ptr->get_read_timestamp(), context4.get_timestamp());
  BOOST_CHECK_EQUAL(delta_ptr->get_read_timestamp(), context4.get_timestamp());
  BOOST_CHECK_EQUAL(delta_ptr2->get_read_timestamp(), context4.get_timestamp());
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
      auto record_ptr = instance.get_bytes_at(record_slot);
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

          auto delta_ptr = instance2.get_bytes_at(delta_slot);

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
      // insert, reading through all versions.
      auto read_context = manager.begin_transaction();
      auto read_result = instance.read(*s, read_context, block_tuple::read_from_delta);

      BOOST_CHECK_EQUAL(read_result->state, 2);
  }
}

BOOST_AUTO_TEST_CASE(storage__insert_abort__remains_latched__success)
{
  const uint64_t size_limit = 1;
  const uint64_t reuse_limit = 1;
  const block_pool_ptr block_store = std::make_shared<block_pool>(size_limit, reuse_limit);

  store<block_mvcc_record> instance{block_store};

  transaction_manager manager;
  auto context = manager.begin_transaction();
  block_mvcc_record record(context);
  auto record_slot = instance.insert(context, record);

  // We need to install the records next, and set the abort actions
  auto record_ptr = instance.get_bytes_at(record_slot);
  record_ptr->install(context);
  context.register_commit_action([record_ptr, context]()
  {
      // commit to context timestamp
      record_ptr->commit(context, context.get_timestamp());
  });

  BOOST_CHECK(record_ptr->get_next() == block_mvcc_record::no_next);

  // Abort thethe transaction
  context.abort();

  // test record timestamps
  BOOST_CHECK(record_ptr->get_begin_timestamp() == context.get_timestamp());
  BOOST_CHECK(record_ptr->get_end_timestamp() == context.get_timestamp());
  BOOST_CHECK(record_ptr->get_read_timestamp() == none_read);

  // record remains latched as the transaction aborted instead of
  // committing
  BOOST_CHECK(record_ptr->is_latched_by(context));
}

BOOST_AUTO_TEST_CASE(storage__update_abort__undo__success)
{
  const uint64_t size_limit = 1;
  const uint64_t reuse_limit = 1;
  const block_pool_ptr block_store = std::make_shared<block_pool>(size_limit, reuse_limit);

  store<block_mvcc_record> instance{block_store};

  transaction_manager manager;
  auto context = manager.begin_transaction();
  block_mvcc_record record(context);
  BOOST_CHECK_EQUAL(record.get_data().state, 0);
  auto record_slot = instance.insert(context, record);

  // We need to install the records next, and set the commit actions
  auto record_ptr = instance.get_bytes_at(record_slot);
  record_ptr->install(context);
  context.register_commit_action([record_ptr, context]()
  {
      // commit to context timestamp
      record_ptr->commit(context, context.get_timestamp());
  });

  BOOST_CHECK(record_ptr->get_next() == block_mvcc_record::no_next);

  BOOST_CHECK_EQUAL(record_ptr->get_data().state, 0);
  auto read_result = instance.read(record_slot, context, block_tuple::read_from_delta);
  BOOST_REQUIRE_EQUAL(read_result->state, 0);

  // End creating head record
  context.commit();

  // delta storage
  const block_pool_ptr delta_store = std::make_shared<block_pool>(size_limit, reuse_limit);
  store<block_delta_mvcc_record> instance2{delta_store};

  auto context2 = manager.begin_transaction();

  // use constructor, will give us a latched record
  block_delta_mvcc_record delta_record(context2);
  delta_record.get_data().state = 1;
  auto delta_slot = instance2.insert(context2, delta_record);

  auto delta_ptr = instance2.get_bytes_at(delta_slot);

  // register abort action only for record as delta is an insert that
  // will be cleared out by garbage collector
  auto end_ts = record_ptr->get_end_timestamp();
  auto next = record_ptr->get_next();
  context2.register_abort_action([record_ptr, context2, end_ts, next]()
  {
      // reset end timestamp and release latch, that is what commit
      // does
      record_ptr->set_next(next);
      record_ptr->commit(context2, end_ts);
  });

  BOOST_REQUIRE(record_ptr->install_next_version(delta_ptr, context2));
  BOOST_REQUIRE(record_ptr->get_next() != block_mvcc_record::no_next);
  BOOST_REQUIRE(record_ptr->is_latched_by(context2));

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
  context2.abort();

  // test record timestamps
  BOOST_REQUIRE_EQUAL(record_ptr->get_begin_timestamp(), context.get_timestamp());
  BOOST_REQUIRE_EQUAL(record_ptr->get_end_timestamp(), context.get_timestamp());
  // we read it to test state was 0
  BOOST_REQUIRE_EQUAL(record_ptr->get_read_timestamp(), context.get_timestamp());
  BOOST_REQUIRE(!record_ptr->is_latched_by(context));
  BOOST_REQUIRE(!record_ptr->is_latched_by(context2));
  BOOST_REQUIRE_EQUAL(record_ptr->get_next(), block_mvcc_record::no_next);

  // Then we read the records using the slot returned from first
  // insert and verify that the update is not readable
  auto context3 = manager.begin_transaction();
  read_result = instance.read(record_slot, context3, block_tuple::read_from_delta);
  BOOST_REQUIRE_EQUAL(read_result->state, 0);
}

BOOST_AUTO_TEST_SUITE_END()
