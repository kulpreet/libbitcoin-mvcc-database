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
#ifndef LIBBITCOIN_MVCC_DATABASE_BLOCK_DATABASE_HPP
#define LIBBITCOIN_MVCC_DATABASE_BLOCK_DATABASE_HPP

#include <atomic>
#include <cstddef>

#include <bitcoin/system.hpp>
#include <bitcoin/database/define.hpp>

#include <bitcoin/database/mvto/accessor.hpp>
#include <bitcoin/database/storage/storage.hpp>
#include <bitcoin/database/transaction_management/transaction_context.hpp>
#include <bitcoin/database/tuples/block_tuple.hpp>
#include <bitcoin/database/tuples/block_tuple_delta.hpp>
#include <bitcoin/database/tuples/mvcc_record.hpp>

#include <libcuckoo/cuckoohash_map.hh>

namespace libbitcoin {
namespace database {

using namespace mvto;
using namespace tuples;
using namespace storage;

/// index by height
typedef
libcuckoo::cuckoohash_map<size_t, slot> height_index_map;

/// index by block hash
typedef
libcuckoo::cuckoohash_map<system::hash_digest, slot> hash_digest_index_map;

typedef
std::shared_ptr<storage::store<block_mvcc_record>> block_store_ptr;

typedef
std::shared_ptr<storage::store<block_delta_mvcc_record>> delta_store_ptr;

/// Access block storage
typedef
accessor<block_mvcc_record, block_delta_mvcc_record> block_mvto_accessor;

/// Stores block_headers each with a list of transaction indexes.
/// Lookup possible by hash or height.
class BCD_API block_database
{
public:
    /// Construct the database.
    block_database(uint64_t, uint64_t, uint64_t, uint64_t);

    /// TODO: Take a snapshot.
    /// TODO: Free all used memory - requires us to switch to object
    /// pool first.
    ~block_database() = default;

    // Startup and shutdown.
    // ------------------------------------------------------------------------

    /// Initialize a new block database.
    bool create();

    /// Call before using the database.
    bool open();

    /// Commit latest inserts.
    void commit();

    /// Compact and write latest snapshot
    bool close();

    // Queries.
    //-------------------------------------------------------------------------

    /// The height of the highest candidate|confirmed block.
    bool top(transaction_context& context, size_t& out_height,
        bool candidate) const;

    // /// Fetch block by block|header index height.
    block_tuple_ptr get(transaction_context& context, size_t height,
        bool candidate) const;

    // /// Fetch block by hash.
    block_tuple_ptr get(transaction_context& context,
        const system::hash_digest& hash) const;

    /// get error from the state field of block_tuple_ptr
    code get_error(block_tuple_ptr) const;

    /// Populate header metadata for the given header.
    void get_header_metadata(transaction_context& context,
        const system::chain::header& header) const;

    // Writers.
    // ------------------------------------------------------------------------

    /// Store header, validated at height, candidate, pending (but
    /// unindexed).
    /// Return memory location where data is stored and null_ptr on error.
    bool store(transaction_context& context,
        const system::chain::header& header,
        const size_t height, const uint32_t median_time_past,
        const uint32_t checksum, const uint8_t state);

    /// Populate pooled block transaction references, state is unchanged.
    bool update_transactions(transaction_context& context,
        const system::chain::block& block);

    /// Promote pooled block to valid|invalid and set code.
    bool validate(transaction_context& context,
        const system::hash_digest& hash, const system::code& error);

    /// Promote pooled|candidate block to candidate|confirmed
    /// respectively.
    bool promote(transaction_context& context,const system::hash_digest& hash,
        size_t height, bool candidate);

    /// Demote candidate|confirmed header to pooled|pooled (not candidate).
    bool demote(transaction_context& context, const system::hash_digest& hash,
        size_t height, bool candidate);

private:
    block_pool_ptr block_store_pool_;
    block_store_ptr block_store_;

    block_pool_ptr delta_store_pool_;
    delta_store_ptr delta_store_;

    block_mvto_accessor accessor_;

    // Blockchain first stores which puts block in
    // candidate_index.  Later block is promoted to candidate and from
    // there can be demoted to pool. A block is never moved from
    // candidate to confirmed, so we don't need to update two indexes
    // in the same transaction.

    // indexes
    std::shared_ptr<height_index_map> candidate_index_;
    std::shared_ptr<height_index_map> confirmed_index_;
    std::shared_ptr<hash_digest_index_map> hash_digest_index_;
};

} // namespace database
} // namespace libbitcoin

#endif
