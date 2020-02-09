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
#include <bitcoin/database/tuples/block_tuple.hpp>
#include <bitcoin/database/tuples/block_tuple_delta.hpp>
#include <bitcoin/database/tuples/mvcc_record.hpp>

#include <libcuckoo/cuckoohash_map.hh>

namespace libbitcoin {
namespace database {

using namespace bc::database::tuples;

// block delta record wrapped in mvcc record
using delta_mvcc_record =
    mvcc_record<block_delta_ptr, block_delta_ptr>;

// block tuple wrapped in mvcc record
using block_mvcc_record =
    mvcc_record<block_tuple_ptr, delta_mvcc_record, block_reader, block_writer>;

// memory allocator for block mvcc record
using block_tuple_memory_store = storage::memory<block_mvcc_record>;

// memory allocator for block delta mvcc record
using block_delta_memory_store = storage::memory<delta_mvcc_record>;

// index for block mvcc record
using height_index_map = libcuckoo::cuckoohash_map<size_t, block_mvcc_record>;
using hash_digest_index_map =
    libcuckoo::cuckoohash_map<system::hash_digest, block_mvcc_record>;

/// Stores block_headers each with a list of transaction indexes.
/// Lookup possible by hash or height.
class BCD_API block_database
{
public:
    /// Construct the database.
    block_database(block_tuple_memory_store&, block_delta_memory_store&);

    /// TODO: Take a snapshot.
    /// TODO: Free all used memory - requires us to switch to object
    /// pool first.
    ~block_database() = default;

    // Startup and shutdown.
    // ------------------------------------------------------------------------

    /// Initialize a new transaction database.
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
    bool top(size_t& out_height, bool candidate) const;

    // /// Fetch block by block|header index height.
    block_tuple_ptr get(size_t height, bool candidate) const;

    // /// Fetch block by hash.
    block_tuple_ptr get(const system::hash_digest& hash) const;

    /// Populate header metadata for the given header.
    void get_header_metadata(const system::chain::header& header) const;

    // Writers.
    // ------------------------------------------------------------------------

    /// Store header, validated at height, candidate, pending (but
    /// unindexed).
    /// Return memory location where data is stored and null_ptr on error.
    block_tuple_ptr store(const system::chain::header& header,
        const size_t height, const uint32_t median_time_past,
        const uint32_t checksum, const uint8_t state);

    /// Populate pooled block transaction references, state is unchanged.
    bool update_transactions(const system::chain::block& block);

    /// Promote pooled block to valid|invalid and set code.
    bool validate(const system::hash_digest& hash, const system::code& error);

    /// Promote pooled|candidate block to candidate|confirmed
    /// respectively.
    bool promote(const system::hash_digest& hash, size_t height, bool candidate);

    /// Demote candidate|confirmed header to pooled|pooled (not candidate).
    bool demote(const system::hash_digest& hash, size_t height,
        bool candidate);

private:
    block_tuple_memory_store& master_memory_store_;
    block_delta_memory_store& delta_memory_store_;

    // Blockchain first stores which puts block in
    // candidate_index.  Later block is promoted to candidate and from
    // there can be demoted to pool. A block is never moved from
    // candidate to confirmed, so we don't need to update two indexes
    // in the same transaction.

    // indexes
    height_index_map candidate_index_;
    height_index_map confirmed_index_;
    hash_digest_index_map hash_digest_index_;

    // tracking top
    std::atomic<size_t> candidate_top_;
    std::atomic<size_t> confirmed_top_;
};

} // namespace database
} // namespace libbitcoin

#endif
