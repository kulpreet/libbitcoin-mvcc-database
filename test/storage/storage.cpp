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

  store<int64_t> instance{block_store};
}

BOOST_AUTO_TEST_CASE(storage__insert__record__success)
{
  const uint64_t size_limit = 1;
  const uint64_t reuse_limit = 1;
  const block_pool_ptr block_store = std::make_shared<block_pool>(size_limit, reuse_limit);

  store<block_mvcc_record> instance{block_store};

  transaction_manager manager;
  auto context = manager.begin_transaction();
  block_mvcc_record record(context);
  instance.insert(context, record);
}

BOOST_AUTO_TEST_SUITE_END()
