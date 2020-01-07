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
#ifndef LIBBITCOIN_DATABASE_SPIN_LATCH_HPP
#define LIBBITCOIN_DATABASE_SPIN_LATCH_HPP

#include <boost/atomic.hpp>
#include <bitcoin/system.hpp>
#include <bitcoin/database/define.hpp>

namespace libbitcoin {
namespace database {

class spinlatch
{
public:

    spinlatch() : state_(unlocked) {}

    void lock()
    {
        while (state_.exchange(locked, boost::memory_order_acquire) == locked) {
            /* busy-wait */
        }
    }
    void unlock()
    {
        state_.store(unlocked, boost::memory_order_release);
    }

private:
    typedef enum
    {
        locked,
        unlocked
    } lockstate;
    boost::atomic<lockstate> state_;
};

class scopedspinlatch
{
public:

    scopedspinlatch(std::shared_ptr<spinlatch> latch) : spinlatch_(latch)
    {
        spinlatch_->lock();
    }

    ~scopedspinlatch()
    {
        spinlatch_->unlock();
    }

    scopedspinlatch(const scopedspinlatch &) = delete;
    scopedspinlatch &operator=(const scopedspinlatch &) = delete;
    scopedspinlatch(scopedspinlatch &&) = delete;
    scopedspinlatch &operator=(scopedspinlatch &&) = delete;

private:
    std::shared_ptr<spinlatch> spinlatch_;
};

}
}

#endif
