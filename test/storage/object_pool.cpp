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
#include <boost/test/unit_test.hpp>

#include <bitcoin/database/transaction_management/transaction_manager.hpp>

#include <atomic>
#include <unordered_set>
#include <vector>

#include <bitcoin/database/storage/object_pool.hpp>

using namespace bc;
using namespace bc::database;
using namespace bc::database::storage;

BOOST_AUTO_TEST_SUITE(object_pool_tests)

// Rather minimalistic checks for whether we reuse memory
BOOST_AUTO_TEST_CASE(object_pool__simple_test__reuse____success)
{
  const uint32_t repeat = 10;
  const uint64_t size_limit = 1;
  const uint64_t reuse_limit = 1;
  object_pool<uint32_t> instance(size_limit, reuse_limit);

  // Put a pointer on the the reuse queue
  uint32_t *reused_ptr = instance.get();
  instance.release(reused_ptr);

  for (uint32_t i = 0; i < repeat; i++) {
    BOOST_CHECK_EQUAL(instance.get(), reused_ptr);
    instance.release(reused_ptr);
  }
}

// Reset the size of the object pool
BOOST_AUTO_TEST_CASE(object_pool__reset_limit______success)
{
  const uint32_t repeat = 10;
  const uint64_t size_limit = 10;
  for (uint32_t iteration = 0; iteration < repeat; ++iteration) {
    object_pool<uint32_t> tested(size_limit, size_limit);
    std::unordered_set<uint32_t *> used_ptrs;

    // The reuse_queue should have a size of size_limit
    for (uint32_t i = 0; i < size_limit; ++i) used_ptrs.insert(tested.get());
    for (auto &it : used_ptrs) tested.release(it);

    tested.set_reuse_limit(size_limit / 2);
    BOOST_CHECK(tested.set_size_limit(size_limit / 2));

    std::vector<uint32_t *> ptrs;
    for (uint32_t i = 0; i < size_limit / 2; ++i) {
      // the first half should be reused pointers
      uint32_t *ptr = tested.get();
    BOOST_CHECK(used_ptrs.find(ptr) != used_ptrs.end());

      // store the pointer to free later
      ptrs.emplace_back(ptr);
    }

    // Should get an exception
    BOOST_CHECK_THROW(tested.get(), no_more_object_exception);

    // free memory
    for (auto &it : ptrs) tested.release(it);
  }
}

// class ObjectPoolTestType {
//  public:
//   ObjectPoolTestType *Use(uint32_t thread_id) {
//     user_ = thread_id;
//     return this;
//   }

//   ObjectPoolTestType *Release(uint32_t thread_id) {
//     // Nobody used this
//     EXPECT_EQ(thread_id, user_);
//     return this;
//   }

//  private:
//   std::atomic<uint32_t> user_;
// };

// // This test generates random workload and sees if the pool gives out
// // the same pointer to two threads at the same time.
// // NOLINTNEXTLINE
// TEST(ObjectPoolTests, ConcurrentCorrectnessTest) {
//   const uint64_t size_limit = 100;
//   const uint64_t reuse_limit = 100;
//   common::ObjectPool<ObjectPoolTestType> tested(size_limit, reuse_limit);
//   auto workload = [&](uint32_t tid) {
//     std::uniform_int_distribution<uint64_t> size_dist(1, reuse_limit);

//     // Randomly generate a sequence of use-free
//     std::default_random_engine generator;
//     // Store the pointers we use.
//     std::vector<ObjectPoolTestType *> ptrs;
//     auto allocate = [&] {
//       try {
//         ptrs.push_back(tested.Get()->Use(tid));
//       } catch (common::NoMoreObjectException &) {
//         // Since threads are alloc and free in random order, object pool could possibly have no object to hand out.
//         // When this occurs, we just do nothing. The purpose of this test is to test object pool concurrently and
//         // check correctness. We just skip and do nothing. The object pool will eventually have objects when other
//         // threads release objects.
//       }
//     };
//     auto free = [&] {
//       if (!ptrs.empty()) {
//         auto pos = RandomTestUtil::UniformRandomElement(&ptrs, &generator);
//         tested.Release((*pos)->Release(tid));
//         ptrs.erase(pos);
//       }
//     };
//     auto set_reuse_limit = [&] { tested.SetReuseLimit(size_dist(generator)); };

//     auto set_size_limit = [&] { tested.SetSizeLimit(size_dist(generator)); };

//     RandomTestUtil::InvokeWorkloadWithDistribution({free, allocate, set_reuse_limit, set_size_limit},
//                                                    {0.25, 0.25, 0.25, 0.25}, &generator, 1000);
//     for (auto *ptr : ptrs) tested.Release(ptr->Release(tid));
//   };
//   common::WorkerPool thread_pool(MultiThreadTestUtil::HardwareConcurrency(), {});
//   MultiThreadTestUtil::RunThreadsUntilFinish(&thread_pool, MultiThreadTestUtil::HardwareConcurrency(), workload, 100);
// }

BOOST_AUTO_TEST_SUITE_END()
