#ifndef LIBBITCOIN_MVCC_MEMORY_BLOCKS_HPP
#define LIBBITCOIN_MVCC_MEMORY_BLOCKS_HPP

#include <bitcoin/system.hpp>

namespace libbitcoin {
namespace database {

/*
 * Allocate and free memory for storing tuples.
 *
 * Uses a pool of memory blocks, that are locked by threads when they
 * are writing a tuple in the block.
 *
 * Each block is of fixed size to accomodate type T objects without
 * breaking alignment.
 *
 * Tuples are stored in slots and when tuples are moved to disk, the
 * slot is returned to the list of tuple slots that can be reused for
 * next tuple allocation.
 */
class memory
  : system::noncopyable
{

};

}
}

#endif
