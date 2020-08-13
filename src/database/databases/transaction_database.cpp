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

#include <bitcoin/database/databases/transaction_database.hpp>
#include <bitcoin/database/tuples/transaction_tuple.hpp>

namespace libbitcoin {
namespace database {

using namespace bc::system;
using namespace chain;
using namespace mvto;
using namespace tuples;
using namespace storage;

transaction_database::transaction_database(uint64_t block_size_limit,
    uint64_t block_reuse_limit, uint64_t delta_size_limit,
    uint64_t delta_reuse_limit)
    : transaction_store_pool_(std::make_shared<block_pool>(block_size_limit, block_reuse_limit)),
      transaction_store_(std::make_shared<storage::store<transaction_mvcc_record>>(transaction_store_pool_)),
      delta_store_pool_(std::make_shared<block_pool>(delta_size_limit, delta_reuse_limit)),
      delta_store_(std::make_shared<storage::store<transaction_delta_mvcc_record>>(delta_store_pool_)),
      accessor_(transaction_mvto_accessor{transaction_store_, delta_store_}),
      transaction_hash_index_(std::make_shared<transaction_hash_index_map>()),
      block_transactions_index_(std::make_shared<block_transactions_index_map>())
{
}

} // namespace database
} // namespace libbitcoin
