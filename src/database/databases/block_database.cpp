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

#include <bitcoin/database/block_state.hpp>
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
      candidate_index_(std::make_shared<height_index_map>()),
      confirmed_index_(std::make_shared<height_index_map>()),
      hash_digest_index_(std::make_shared<hash_digest_index_map>())
{
}

bool block_database::top(transaction_context& context, size_t& out_height,
    bool candidate) const
{
    auto index = candidate ? candidate_index_ : confirmed_index_;
    auto size = index->size();

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

    hash_digest_index_->insert(header.hash(), result_slot);
    return true;
}

// Find the slot from the hash_digest index and
// find the readable version for the transaction timestamp
block_tuple_ptr block_database::get(transaction_context& context,
    const system::hash_digest& hash) const
{
    try
    {
        auto at_slot = hash_digest_index_->find(hash);
        return accessor_.get(context, at_slot, block_tuple::read_from_delta);
    }
    catch (std::out_of_range e)
    {
        context.abort();
        return nullptr;
    }
}

static uint8_t update_validation_state(uint8_t original, bool positive)
{
    // May only validate or invalidate an unvalidated block.
    BITCOIN_ASSERT(!is_failed(original) && !is_valid(original));

    // Preserve the confirmation state.
    const auto confirmation_state = original & block_state::confirmations;
    const auto validation_state = positive ? block_state::valid :
        block_state::failed;

    // Merge the new validation state with existing confirmation state.
    return confirmation_state | validation_state;
}

static uint8_t update_confirmation_state(uint8_t original, bool positive,
    bool candidate)
{
    // May only confirm a valid block.
    BITCOIN_ASSERT(!positive || candidate || is_valid(original));

    // May only unconfirm a confirmed block.
    BITCOIN_ASSERT(positive || candidate || is_confirmed(original));

    // May only candidate an unfailed block.
    BITCOIN_ASSERT(!positive || !candidate || !is_failed(original));

    // May only uncandidate a candidate header.
    BITCOIN_ASSERT(positive || !candidate || is_candidate(original));

    // Preserve the validation state (header-indexed blocks can be pent).
    const auto validation_state = original & block_state::validations;
    const auto positive_state = candidate ? block_state::candidate :
        block_state::confirmed;

    // Deconfirmation is always directly to the pooled state.
    const auto confirmation_state = positive ? positive_state :
        block_state::missing;

    // Merge the new confirmation state with existing validation state.
    return confirmation_state | validation_state;
}

// Find the slot from the candidate|confirmed index and
// find the readable version for the transaction timestamp
block_tuple_ptr block_database::get(transaction_context& context,
    size_t height, bool candidate) const
{
    auto index = candidate ? candidate_index_ : confirmed_index_;
    try
    {
        auto at_slot = index->find(height);
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
    const system::hash_digest &hash, size_t height, bool candidate,
    bool promote_or_demote)
{
    block_tuple_ptr read_block;
    slot at_slot;

    try
    {
        hash_digest_index_->find(hash, at_slot);
        read_block = accessor_.get(context, at_slot, block_tuple::read_from_delta);
    }
    catch (std::out_of_range e)
    {
        context.abort();
        return false;
    }

    auto original = read_block->state;
    const auto updated_state = update_confirmation_state(original, promote_or_demote, candidate);

    auto delta_data = std::make_shared<block_tuple_delta>();
    delta_data->state = updated_state;
    if (!accessor_.update(context, at_slot, delta_data))
    {
        context.abort();
        return false;
    }

    auto index = candidate ? candidate_index_ : confirmed_index_;

    // add to the selected index - promote
    if (promote_or_demote)
        return index->insert(height, at_slot);

    // remove from the selected index - demote
    return index->erase(height);
}

bool block_database::promote(transaction_context& context,
    const system::hash_digest &hash, size_t height, bool candidate)
{
    return promote(context, hash, height, candidate, true);
}

bool block_database::demote(transaction_context& context,
    const system::hash_digest& hash, size_t height, bool candidate)
{
    return promote(context, hash, height, candidate, false);
}

code block_database::get_error(block_tuple_ptr block) const
{
    // Checksum stores error code if the block is invalid.
    return is_failed(block->state) ? static_cast<error::error_code_t>(block->checksum) :
        error::success;
}

void block_database::get_header_metadata(transaction_context& context,
    const chain::header& header) const
{
    auto read_block = get(context, header.hash());
    if (read_block == nullptr)
        return;

    const auto state = read_block->state;
    header.metadata.exists = true;
    header.metadata.error = get_error(read_block);
    header.metadata.candidate = is_candidate(state);
    header.metadata.confirmed = is_confirmed(state);
    header.metadata.validated = is_valid(state) || is_failed(state);
    // header.metadata.populated = transaction_count() != 0;
    header.metadata.median_time_past = read_block->median_time_past;
}

bool block_database::validate(transaction_context& context,
    const system::hash_digest& hash, const system::code& error)
{
    block_tuple_ptr read_block;
    slot at_slot;

    try
    {
        hash_digest_index_->find(hash, at_slot);
        read_block = accessor_.get(context, at_slot, block_tuple::read_from_delta);
    }
    catch (std::out_of_range e)
    {
        context.abort();
        return false;
    }

    auto original = read_block->state;
    const auto updated_state = update_validation_state(original, !error);

    auto delta_data = std::make_shared<block_tuple_delta>();
    delta_data->state = updated_state;
    if (!accessor_.update(context, at_slot, delta_data))
    {
        context.abort();
        return false;
    }
    return true;
}


} // namespace database
} // namespace libbitcoin
