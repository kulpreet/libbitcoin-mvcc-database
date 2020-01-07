#ifndef LIBBITCOIN_MVCC_MEMORY_BLOCKS_HPP
#define LIBBITCOIN_MVCC_MEMORY_BLOCKS_HPP

#include <bitcoin/system.hpp>

namespace libbitcoin {
namespace database {

/**
 * Tracks a memory pool of sepcified size.
 *
 * Uses boost object pool to avoid repeatedly
 * allocating memory.
 *
 * The memory store is templatized for record layout.
 *
 * We don't templatize the allocator unless required later.
 */
template <typename RecordLayout>
class MemoryStore
  : system::noncopyable
{

};

}
}

#endif
