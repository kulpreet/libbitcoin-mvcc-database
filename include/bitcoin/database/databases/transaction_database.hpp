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
#ifndef LIBBITCOIN_MVCC_DATABASE_TRANSACTION_DATABASE_HPP
#define LIBBITCOIN_MVCC_DATABASE_TRANSACTION_DATABASE_HPP

#include <atomic>
#include <cstddef>

#include <bitcoin/system.hpp>
#include <bitcoin/database/define.hpp>

#include <bitcoin/database/mvto/accessor.hpp>
#include <bitcoin/database/storage/storage.hpp>
#include <bitcoin/database/transaction_management/transaction_context.hpp>
#include <bitcoin/database/tuples/transaction_tuple.hpp>
#include <bitcoin/database/tuples/transaction_tuple_delta.hpp>
#include <bitcoin/database/tuples/mvcc_record.hpp>

#include <libcuckoo/cuckoohash_map.hh>

namespace libbitcoin {
namespace database {

using namespace mvto;
using namespace tuples;
using namespace storage;

/// index by height
typedef
libcuckoo::cuckoohash_map<size_t, std::vector<system::hash_digest>> block_transactions_index_map;

/// index by transaction hash
typedef
libcuckoo::cuckoohash_map<system::hash_digest, slot> transaction_hash_index_map;

/// index by block hash.
/// We don't index this directly to slot, as then we have to update
/// another index when transaction is moved from memory to disk.
typedef
libcuckoo::cuckoohash_map<system::hash_digest, std::vector<system::hash_digest>>
block_hash_index_map;

typedef
std::shared_ptr<storage::store<transaction_mvcc_record>> transaction_store_ptr;

typedef
std::shared_ptr<storage::store<transaction_delta_mvcc_record>> delta_store_ptr;

/// Access transaction storage
typedef
accessor<transaction_mvcc_record, transaction_delta_mvcc_record> transaction_mvto_accessor;

class BCD_API transaction_database
{
public:
    /// Construct the database.
    transaction_database(uint64_t block_size_limit, uint64_t block_reuse_limit,
        uint64_t delta_size_limit, uint64_t delta_reuse_limit);

    /// TODO: Take a snapshot.
    /// TODO: Free all used memory - requires us to switch to object
    /// pool first.
    ~transaction_database() = default;

    // Queries.
    //-------------------------------------------------------------------------

    /// Fetch transaction by its hash.
    transaction_tuple_ptr get(const system::hash_digest& hash) const;

    /// Populate tx metadata for the given block context.
    void get_block_metadata(const system::chain::transaction& tx,
        uint32_t forks, size_t fork_height) const;

    /// Populate tx metadata for the given transaction pool context.
    void get_pool_metadata(const system::chain::transaction& tx,
        uint32_t forks) const;

    /// Populate output metadata for the specified point and fork point.
    bool get_output(const system::chain::output_point& point,
        size_t fork_height) const;

    // Writers.
    // ------------------------------------------------------------------------

    /// Store a transaction not associated with a block.
    bool store(const system::chain::transaction& tx, uint32_t forks);

    /// Store a set of transactions (potentially from an unconfirmed block).
    bool store(const system::chain::transaction::list& transactions);

    /// Mark outputs spent by the candidate tx.
    bool candidate(const system::hash_digest& hash);

    /// Unmark outputs formerly spent by the candidate tx.
    bool uncandidate(const system::hash_digest& hash);

    /// Promote the transaction to confirmed (uncached).
    bool confirm(const system::hash_digest& hash, size_t height,
        uint32_t median_time_past, size_t position);

    /// Promote the set of transactions associated with a block to confirmed.
    bool confirm(const system::chain::block& block, size_t height,
        uint32_t median_time_past);

    /// Demote the set of transactions associated with a block to pooled.
    bool unconfirm(const system::chain::block& block);

private:
    block_pool_ptr transaction_store_pool_;
    transaction_store_ptr transaction_store_;

    block_pool_ptr delta_store_pool_;
    delta_store_ptr delta_store_;

    transaction_mvto_accessor accessor_;

    // indexes
    // transaction hash to slot index
    std::shared_ptr<transaction_hash_index_map> transaction_hash_index_;
    // block hash to transaction hashes
    std::shared_ptr<block_transactions_index_map> block_transactions_index_;
};

} // namespace database
} // namespace libbitcoin

#endif
