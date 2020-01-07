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
#ifndef LIBBITCOIN_DATABASE_TRANSACTION_CONTEXT_HPP
#define LIBBITCOIN_DATABASE_TRANSACTION_CONTEXT_HPP

#include <bitcoin/system.hpp>
#include <bitcoin/database/define.hpp>

typedef uint64_t timestamp_t;

namespace libbitcoin {
namespace database {

// transaction states, for now, active and committed are place
// holders.
enum class state
{
    active,
    committed
};

/// transaction_context captures the transaction id, and will need to
/// capture the state of the transaction as well.
class BCD_API transaction_context
{
public:

    /// Constructor
    transaction_context(timestamp_t timestamp, state state)
        : timestamp_(timestamp), state_(state)
    {
    }

    timestamp_t get_timestamp() const
    {
        return timestamp_;
    }

    state get_state() const
    {
        return state_;
    }

    void set_state(state to)
    {
        state_ = to;
    }

private:
    timestamp_t timestamp_;
    state state_;
};

} // namespace database
} // namespace libbitcoin


#endif
