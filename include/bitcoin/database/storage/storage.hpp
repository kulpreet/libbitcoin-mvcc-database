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

#ifndef LIBBITCOIN_MVCC_STORAGE_HPP
#define LIBBITCOIN_MVCC_STORAGE_HPP

#include <atomic>
#include <bitcoin/system.hpp>

#include <bitcoin/database/transaction_management/spinlatch.hpp>
#include <bitcoin/database/transaction_management/transaction_context.hpp>
#include <bitcoin/database/storage/raw_block.hpp>
#include <bitcoin/database/storage/object_pool.hpp>
#include <bitcoin/database/storage/slot.hpp>
#include <bitcoin/database/storage/slot_iterator.hpp>
#include <bitcoin/database/container/concurrent_bitmap.hpp>

namespace libbitcoin {
namespace database {
namespace storage {

using namespace container;

template <typename record>
class store
{
public:
    /**
     * Constructs a new storage for the give type record, using the
     * given block_stre as the source of its storage blocks.
     *
     * @param store the block store to use.
     */
    store(const block_pool_ptr);

    /**
     * Destructs store, releases all its blocks to block pool
     */
    ~store();

    /**
     * Inserts a record, and update the version chain the link to the
     * given delta record. The slot allocated for the record is
     * returned.
     *
     * @param txn the calling transaction
     * @param data reference to a record on stack. A copy be stored in
     * this store, after which the record on stack can be deleted.
     * @return the slot allocated for this insert, used to
     * identify this record's physical location for indexes.
     */
    slot insert(transaction_context&, const record &);

    // Given a slot and a transaction context, read the entire version
    // chain, build the final state of the mvcc version chain into a
    // single mvcc record and return it. The record does not
    // correspond to the memory in raw block memory object pool.
    typename record::tuple_ptr read(const slot&, const transaction_context&,
        typename record::reader) const;

    raw_block* get_current_block();

    record* get_bytes_at(const slot&) const;

private:

    // get a new block from block pool
    raw_block* get_new_block();

    // Initialize a raw block
    void initialize_raw_block(raw_block*);

    // Read the slot bitmap from the raw block contents
    raw_concurrent_bitmap* get_slot_bitmap(raw_block*);

    // allocate a slot in raw block, set slot* to the new memory
    // location in raw block
    bool allocate_in(raw_block*, slot*);

    // insert record into slot with given transaction context.
    void insert_into(transaction_context&, const record&, const slot&);

    // move insertion_header forward, getting new block from pool, if
    // required
    void check_move_head(std::list<raw_block *>::iterator);

    block_pool_ptr block_pool_;
    std::list<raw_block*> blocks_;
    std::shared_ptr<spinlatch> blocks_latch_;
    std::shared_ptr<spinlatch> insert_head_latch_;

    std::list<raw_block*>::iterator insertion_head_;

    uint32_t record_size_;
    uint32_t num_slots_in_block_;
};

} // namespace storage
} // namespace database
} // namespace libbitcoin

#include <bitcoin/database/impl/storage.ipp>

#endif
