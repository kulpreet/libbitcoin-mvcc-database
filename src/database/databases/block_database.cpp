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
      accessor_(block_mvto_accessor{block_store_, delta_store_}),
      candidate_top_(0), confirmed_top_(0)
{
}

bool block_database::top(size_t& out_height, bool candidate) const
{
    out_height = candidate ? candidate_top_.load() : confirmed_top_.load();
    if (out_height == 0)
        return false;
    return true;
}

// // block_tuple_ptr block_database::store(const system::chain::header& header,
// //     const size_t height, const uint32_t median_time_past,
// //     const uint32_t checksum, const uint8_t state)
// // {
// //     // // get memory using the memory store
// //     // auto memory_ptr = master_memory_store_.allocate();

// //     // if (memory_ptr == nullptr)
// //     //     return nullptr;

// //     // // set header data
// //     // memory_ptr->previous_block_hash = header.previous_block_hash();
// //     // memory_ptr->merkle_root = header.merkle_root();
// //     // memory_ptr->version = header.version();
// //     // memory_ptr->timestamp = header.timestamp();
// //     // memory_ptr->bits = header.bits();
// //     // memory_ptr->nonce = header.nonce();

// //     // // set block data
// //     // memory_ptr->height = height;
// //     // memory_ptr->median_time_past = median_time_past;
// //     // memory_ptr->checksum = checksum;
// //     // memory_ptr->state = state;

// //     // hash_digest_index_.insert(header.hash(), memory_ptr);
// //     // return memory_ptr;
// // }

// // // Find the block from the hash_digest index and
// // // find the readable version for the transaction timestamp
// // block_tuple_ptr block_database::get(const system::hash_digest& hash) const
// // {
// //     // auto block_tuple = hash_digest_index_.find(hash);
// //     // return block_tuple;
// // }

// // // Find block from block hash index and then update it.
// // // The update won't be visible until the transaction is committed
// // bool block_database::promote(const system::hash_digest &hash, size_t height,
// //     bool candidate)
// // {
// //     // auto block_ptr = hash_digest_index_.find(hash);
// //     // if (candidate) {
// //     // }
// // }

} // namespace database
} // namespace libbitcoin
