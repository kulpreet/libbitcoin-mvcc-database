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

#include <random>
#include <boost/test/unit_test.hpp>
#include <bitcoin/database/container/concurrent_bitmap.hpp>

using namespace bc;
using namespace bc::database;
using namespace bc::database::container;

BOOST_AUTO_TEST_SUITE(concurrent_bitmap_tests)

BOOST_AUTO_TEST_CASE(concurrent_bitmap__set_busy_status__not_set__success)
{
    std::default_random_engine generator;
    // Number of bitmap sizes to test.
    const uint32_t num_bitmap_sizes = 50;
    // Maximum bitmap size.
    const uint32_t max_bitmap_size = 1000;

    for (uint32_t iter = 0; iter < num_bitmap_sizes; ++iter) {
        auto num_elements = std::uniform_int_distribution<uint32_t>(1U, max_bitmap_size)(generator);
        raw_concurrent_bitmap *bitmap = raw_concurrent_bitmap::allocate(num_elements);

        // Verify bitmap initialized to all 0s
        for (uint32_t i = 0; i < num_elements; ++i) {
            BOOST_CHECK(!bitmap->test(i));
        }

        // Randomly permute bitmap and STL bitmap and compare equality
        std::vector<bool> stl_bitmap = std::vector<bool>(num_elements);
        for (uint32_t i = 0; i < num_elements; ++i) {
        BOOST_CHECK_EQUAL(stl_bitmap[i], (*bitmap)[i]);
        }

        uint32_t num_iterations = 32;
        std::default_random_engine generator;
        for (uint32_t i = 0; i < num_iterations; ++i) {
            auto element = std::uniform_int_distribution<uint32_t>(0, static_cast<int>(num_elements - 1))(generator);
            BOOST_CHECK(bitmap->flip(element, bitmap->test(element)));
            stl_bitmap[element] = !stl_bitmap[element];
            for (uint32_t i = 0; i < num_elements; ++i) {
                BOOST_CHECK_EQUAL(stl_bitmap[i], (*bitmap)[i]);
            }
        }

        // Verify that flip fails if expected_val doesn't match current value
        auto element = std::uniform_int_distribution<int>(0, static_cast<int>(num_elements - 1))(generator);
        BOOST_CHECK(!bitmap->flip(element, !bitmap->test(element)));
        raw_concurrent_bitmap::deallocate(bitmap);
  }
}

BOOST_AUTO_TEST_CASE(concurrent_bitmap__first_unset_pos__set__success)
{
    std::default_random_engine generator;
    // Number of bitmap sizes to test.
    const uint32_t num_bitmap_sizes = 50;
    // Maximum bitmap size.
    const uint32_t max_bitmap_size = 1000;
    uint32_t pos = 0;

    // test a wide range of sizes
    for (uint32_t iter = 0; iter < num_bitmap_sizes; ++iter) {
        auto num_elements = std::uniform_int_distribution<uint32_t>(1U, max_bitmap_size)(generator);
        raw_concurrent_bitmap *bitmap = raw_concurrent_bitmap::allocate(num_elements);

        // should return false if we start searching out of range
        BOOST_CHECK(!bitmap->first_unset_pos(num_elements, num_elements, &pos));
        BOOST_CHECK(!bitmap->first_unset_pos(num_elements, num_elements + 1, &pos));

        // as we flip bits from start to end, verify that the position of the next unset pos is correct
        for (uint32_t i = 0; i < num_elements; ++i) {
            BOOST_CHECK(bitmap->first_unset_pos(num_elements, 0, &pos));
            BOOST_CHECK_EQUAL(pos, i);
            BOOST_CHECK(bitmap->flip(i, false));
        }
        // once the bitmap is full, we should not be able to find an unset bit
        BOOST_CHECK(!bitmap->first_unset_pos(num_elements, 0, &pos));

        raw_concurrent_bitmap::deallocate(bitmap);
    }

  // manual targeted test for specific bits
  {
    const uint32_t num_elements = 16;
    raw_concurrent_bitmap *bitmap = raw_concurrent_bitmap::allocate(num_elements);

    // x = set, _ = unset
    // fill everything, resulting in x x x
    for (uint32_t i = 0; i < num_elements; ++i) {
      BOOST_CHECK(bitmap->flip(i, false));
    }

    // once the bitmap is full, we should not be able to find an unset bit
    BOOST_CHECK(!bitmap->first_unset_pos(num_elements, 0, &pos));

    // try to find specific unset bits, x = set, _ = unset
    uint32_t flip_idx[3] = {5, 12, 13};
    // note: there are more than 3 bits
    // x _ x should return middle
    BOOST_CHECK(bitmap->flip(flip_idx[1], true));
    BOOST_CHECK(bitmap->first_unset_pos(num_elements, 0, &pos));
    BOOST_CHECK_EQUAL(pos, flip_idx[1]);
    // _ _ x should return first
    BOOST_CHECK(bitmap->flip(flip_idx[0], true));
    BOOST_CHECK(bitmap->first_unset_pos(num_elements, 0, &pos));
    BOOST_CHECK_EQUAL(pos, flip_idx[0]);
    // _ _ x should return middle if searching from middle
    BOOST_CHECK(bitmap->first_unset_pos(num_elements, flip_idx[1] - 1, &pos));
    BOOST_CHECK_EQUAL(pos, flip_idx[1]);
    BOOST_CHECK(bitmap->first_unset_pos(num_elements, flip_idx[1], &pos));
    BOOST_CHECK_EQUAL(pos, flip_idx[1]);
    // x x _ should return last
    BOOST_CHECK(bitmap->flip(flip_idx[0], false));
    BOOST_CHECK(bitmap->flip(flip_idx[1], false));
    BOOST_CHECK(bitmap->flip(flip_idx[2], true));
    BOOST_CHECK(bitmap->first_unset_pos(num_elements, 0, &pos));
    BOOST_CHECK_EQUAL(pos, flip_idx[2]);
    // x _ _, note we expect idx [1] and [2] to be part of the same word
    BOOST_CHECK(bitmap->flip(flip_idx[1], true));
    BOOST_CHECK(bitmap->first_unset_pos(num_elements, flip_idx[2], &pos));
    BOOST_CHECK_EQUAL(pos, flip_idx[2]);

    raw_concurrent_bitmap::deallocate(bitmap);
  }
}


BOOST_AUTO_TEST_CASE(concurrent_bitmap__first_unset_pos__with_all_set__success)
{
    const uint32_t num_elements = 16;
    raw_concurrent_bitmap *bitmap = raw_concurrent_bitmap::allocate(num_elements);
    uint32_t pos;

    for (uint32_t i = 0; i < num_elements; ++i) {
        BOOST_CHECK(bitmap->flip(i, false));
    }
    BOOST_CHECK(!bitmap->first_unset_pos(num_elements, 0, &pos));

    raw_concurrent_bitmap::deallocate(bitmap);
}

BOOST_AUTO_TEST_CASE(concurrent_bitmap__first_unset_pos__with_all_but_last_set__success)
{
    const uint32_t num_elements = 129;
    raw_concurrent_bitmap *bitmap = raw_concurrent_bitmap::allocate(num_elements);
    uint32_t pos;

    // flip everything but the last bit
    for (uint32_t i = 0; i < num_elements - 1; ++i) {
      BOOST_CHECK(bitmap->flip(i, false));
    }
    BOOST_CHECK(bitmap->test(0));
    // expect the last bit to be ok
    BOOST_CHECK(bitmap->first_unset_pos(num_elements, 0, &pos));
    BOOST_CHECK_EQUAL(pos, 128);

    raw_concurrent_bitmap::deallocate(bitmap);
}

BOOST_AUTO_TEST_SUITE_END()
