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

#include <bitcoin/database/databases/block_database.hpp>
#include <bitcoin/database/tuples/block_tuple.hpp>

namespace libbitcoin {
namespace database {

using namespace bc::system;
using namespace chain;
using namespace mvto;
using namespace tuples;
using namespace storage;

block_database::block_database(uint64_t block_size_limit,
    uint64_t block_reuse_limit, uint64_t delta_size_limit,
    uint64_t delta_reuse_limit)
    : block_store_pool_(std::make_shared<block_pool>(block_size_limit, block_reuse_limit)),
      block_store_(std::make_shared<storage::store<block_mvcc_record>>(block_store_pool_)),
      delta_store_pool_(std::make_shared<block_pool>(delta_size_limit, delta_reuse_limit)),
      delta_store_(std::make_shared<storage::store<block_delta_mvcc_record>>(delta_store_pool_)),
      accessor_(block_mvto_accessor{block_store_, delta_store_})
{
}

bool block_database::top(transaction_context& context, size_t& out_height,
    bool candidate) const
{
    auto index = candidate ? candidate_index_ : confirmed_index_;
    auto size = index.size();

    if (size == 0)
    {
        context.abort();
        return false;
    }

    out_height = size - 1;
    return true;
}

bool block_database::store(transaction_context& context,
    const system::chain::header& header,
    const size_t height, const uint32_t median_time_past,
    const uint32_t checksum, const uint8_t state)
{
    auto data = std::make_shared<block_tuple>();
    // set header data
    data->previous_block_hash = header.previous_block_hash();
    data->merkle_root = header.merkle_root();
    data->version = header.version();
    data->timestamp = header.timestamp();
    data->bits = header.bits();
    data->nonce = header.nonce();

    // set block data
    data->height = height;
    data->median_time_past = median_time_past;
    data->checksum = checksum;
    data->state = state;

    auto result_slot = accessor_.put(context, data);

    if (!result_slot)
    {
        context.abort();
        return false;
    }

    hash_digest_index_.insert(header.hash(), result_slot);
    return true;
}

// Find the slot from the hash_digest index and
// find the readable version for the transaction timestamp
block_tuple_ptr block_database::get(transaction_context& context,
    const system::hash_digest& hash) const
{
    try
    {
        auto at_slot = hash_digest_index_.find(hash);
        return accessor_.get(context, at_slot, block_tuple::read_from_delta);
    }
    catch (std::out_of_range e)
    {
        context.abort();
        return nullptr;
    }
}

// Find the slot from the candidate|confirmed index and
// find the readable version for the transaction timestamp
block_tuple_ptr block_database::get(transaction_context& context,
    size_t height, bool candidate) const
{
    auto index = candidate ? candidate_index_ : confirmed_index_;
    try
    {
        auto at_slot = index.find(height);
        return accessor_.get(context, at_slot, block_tuple::read_from_delta);
    }
    catch (std::out_of_range e)
    {
        context.abort();
        return nullptr;
    }
}

// Find block from block hash index and then update it.
// The update won't be visible until the transaction is committed
bool block_database::promote(transaction_context& context,
    const system::hash_digest &hash, size_t height,
    bool candidate)
{
    try
    {
        auto slot = hash_digest_index_.find(hash);
        auto index = candidate ? candidate_index_ : confirmed_index_;
        return index.insert(height, slot);
    }
    catch (std::out_of_range e)
    {
        context.abort();
        return false;
    }
}

} // namespace database
} // namespace libbitcoin
