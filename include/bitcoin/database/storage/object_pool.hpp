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

#ifndef LIBBITCOIN_MVCC_DATABASE_OBJECT_POOL_HPP
#define LIBBITCOIN_MVCC_DATABASE_OBJECT_POOL_HPP

#include <queue>
#include <string>
#include <utility>

#include <bitcoin/system.hpp>
#include <bitcoin/database/storage/allocator.hpp>
#include <bitcoin/database/storage/raw_block.hpp>
#include <bitcoin/database/transaction_management/spinlatch.hpp>

namespace libbitcoin {
namespace database {
namespace storage {

/**
 * An exception thrown by object pools when they reach their size limits and
 * cannot give more memory space for objects.
 */
class no_more_object_exception : public std::exception {
public:
  /**
   * Construct an exception that can be thrown by a object pool
   * @param limit the object pool limit size
   */
  explicit no_more_object_exception(uint64_t limit)
      : message_("Object Pool have no object to hand out. Exceed size limit " +
                 std::to_string(limit) + ".\n") {}
  /**
   * Describe the exception.
   * @return a string of exception description
   */
  const char *what() const noexcept override { return message_.c_str(); }

private:
  std::string message_;
};

/**
 * An exception thrown by object pools when the allocator fails to fetch memory
 * space. This can happen when the caller asks for an object, the object pool
 * doesn't have reusable object and the underlying allocator fails to get new
 * memory due to system running out of memory
 */
class allocator_failure_exception : public std::exception {
public:
  /**
   * Describe the exception.
   * @return a string of exception description
   */
  const char *what() const noexcept override {
    return "allocator fails to allocate memory.\n";
  }
};

/**
 * Object pool for memory allocation.
 *
 * This prevents liberal calls to malloc and new in the code and makes tracking
 * our memory performance easier.
 * @tparam T the type of objects in the pool.
 * @tparam The allocator to use when constructing and destructing a new object.
 *         In most cases it can be left out and the default allocator will
 *         suffice (malloc). If you want richer behavior, define your own
 *         structure to return a pointer that the object pool will then take
 *         control over. The returned pointer will be eventually freed with the
 *         supplied delete method, but its memory location will potentially be
 *         handed out multiple times before that happens.
 */
template <typename T, typename allocator = byte_aligned_allocator<T>>
class object_pool {
public:
  /**
   * Initializes a new object pool with the supplied limit to the number of
   * objects reused.
   *
   * @param size_limit the maximum number of objects the object pool controls
   * @param reuse_limit the maximum number of reusable objects
   */
  object_pool(uint64_t size_limit, uint64_t reuse_limit)
      : latch_{std::make_shared<spinlatch>()}, size_limit_(size_limit),
        reuse_limit_(reuse_limit), current_size_(0) {}

  /**
   * Destructs the memory pool. Frees any memory it holds.
   *
   * Beware that the object pool will not deallocate some piece of memory
   * not explicitly released via a Release call.
   */
  ~object_pool() {
    T *result = nullptr;
    while (!reuse_queue_.empty()) {
      result = reuse_queue_.front();
      alloc_.deallocate(result);
      reuse_queue_.pop();
    }
  }

  /**
   * Returns a piece of memory to hold an object of T.
   * @throw no_more_object_exception if the object pool has reached the limit of
   * how many objects it may hand out.
   * @throw allocator_failure_exception if the allocator fails to return a valid
   * memory address.
   * @return pointer to memory that can hold T
   */
  T *get() {
    scopedspinlatch guard(latch_);
    if (reuse_queue_.empty() && current_size_ >= size_limit_)
      throw no_more_object_exception(size_limit_);
    T *result = nullptr;
    if (reuse_queue_.empty()) {
      result = alloc_.allocate();

      // result could be null because the allocator may not find
      // enough memory space
      if (result != nullptr)
        current_size_++;
    } else {
      result = reuse_queue_.front();
      reuse_queue_.pop();
      alloc_.reuse(result);
    }
    // If result is nullptr. The call to alloc_.allocate() failed
    // (i.e. can't allocate more memory from the system).
    if (result == nullptr)
      throw allocator_failure_exception();
    BITCOIN_ASSERT_MSG(current_size_ <= size_limit_,
                   "Object pool has exceeded its size limit.");
    return result;
  }

  /**
   * Set the object pool's size limit.
   *
   * The operation fails if the object pool has already allocated more objects
   * than the size limit.
   *
   * @param new_size the new object pool size
   * @return true if new_size is successfully set and false the operation fails
   */
  bool set_size_limit(uint64_t new_size) {
    scopedspinlatch guard(latch_);
    if (new_size >= current_size_) {
      // current_size_ might increase and become > new_size if we don't use lock
      size_limit_ = new_size;
      BITCOIN_ASSERT_MSG(current_size_ <= size_limit_,
                     "object pool size exceed its size limit");
      return true;
    }
    return false;
  }

  /**
   * Set the reuse limit to a new value. This function always succeed and
   * immediately changes reuse limit.
   *
   * A reuse limit simply determines the maximum number of reusable objects the
   * object pool should maintain and can be any non-negative number.
   *
   * If reuse limit > size limit. It's still valid.
   * It's just that the number of reusable objects in the pool will never reach
   * reuse limit because # of reusable objects <= current size <= size limit <
   * reuse_limit.
   *
   * If it's 0, then the object pool just never reuse object.
   *
   * @param new_reuse_limit
   */
  void set_reuse_limit(uint64_t new_reuse_limit) {
    scopedspinlatch guard(latch_);
    reuse_limit_ = new_reuse_limit;
    T *obj = nullptr;
    while (reuse_queue_.size() > reuse_limit_) {
      obj = reuse_queue_.front();
      alloc_.deallocate(obj);
      reuse_queue_.pop();
      current_size_--;
    }
  }

  /**
   * Releases the piece of memory given, allowing it to be freed or reused for
   * later. Although the memory is not necessarily immediately reclaimed, it
   * will be unsafe to access after entering this call.
   *
   * @param obj pointer to object to release
   */
  void release(T *obj) {
    BITCOIN_ASSERT_MSG(obj != nullptr, "releasing a null pointer");
    scopedspinlatch guard(latch_);
    if (reuse_queue_.size() >= reuse_limit_) {
      alloc_.deallocate(obj);
      current_size_--;
    } else {
      reuse_queue_.push(obj);
    }
  }

  /**
   * @return size limit of the object pool
   */
  uint64_t get_size_limit() const { return size_limit_; }

private:
    allocator alloc_;
    std::shared_ptr<spinlatch> latch_;
    std::queue<T *> reuse_queue_;

    // the maximum number of objects a object pool can have
    uint64_t size_limit_;

    // the maximum number of reusable objects in reuse_queue
    // current_size_ represents the number of objects the object pool has
    // allocated, including objects that have been given out to callers and those
    // reside in reuse_queue
    uint64_t reuse_limit_;

    uint64_t current_size_;
};

/**
 * A block store is essentially an object pool. However, all blocks should be
 * aligned, so we will need to use the default constructor instead of raw
 * malloc.
 */
typedef object_pool<raw_block, block_allocator> block_pool;
typedef std::shared_ptr<block_pool> block_pool_ptr;

} // namespace storage
} // namespace database
} // namespace libbitcoin

#endif
