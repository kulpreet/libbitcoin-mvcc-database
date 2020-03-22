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

#include <bitcoin/database/transaction_management/spinlatch.hpp>
#include <bitcoin/database/transaction_management/transaction_context.hpp>

namespace libbitcoin {
namespace database {

transaction_context::transaction_context(timestamp_t timestamp, state state)
    : timestamp_(timestamp), state_(state), end_actions_()
{
}

bool transaction_context::is_committed() const
{
    return state_ == state::committed;
}

bool transaction_context::commit()
{
    set_state(state::committed);
    for (auto action = end_actions_.begin(); action != end_actions_.end(); action++)
    {
        (*action)();
    }
    return true;
}

void transaction_context::register_end_action(const transaction_end_action& action)
{
    end_actions_.push_front(action);
}

timestamp_t transaction_context::get_timestamp() const
{
    return timestamp_;
}

state transaction_context::get_state() const
{
    return state_;
}

void transaction_context::set_state(const state to)
{
    state_ = to;
}

} // namespace database
} // namespace libbitcoin
