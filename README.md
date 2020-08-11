# Build

Uses cmake. Requires libbitcoin-system.

`mkdir build && cd build`
`PKG_CONFIG_PATH=path/to/libbitcoin-system/lib/pkgconfig/ cmake -DCMAKE_BUILD_TYPE=Release|Debug ../`
`make && make test`

## Requirements

We need c++17 to enable `alignas` for use in raw memory block pool. I
couldn't make `class alignas(1 << 20) raw_block` work on c++11 or
c++14. Using jemalloc didn't help either.

# Goals

1. In memory multi version concurrency control.
2. Handle databases larger than RAM available.
3. Bias decisions in favour of faster block validation.
4. API Compatibility with libbitcoin-database.

## Background

libbitcoin-database uses mmap and that has lead to some problems in
case of a hard shutdown. Also, depending on the VM subsystem to keep
UTXOs in memory has run into problems. This is highlighted by our need
to build a utxo cache.

The current libbitcoin-database also uses mutexes to obtain locks on
databases before reading or writing to the database. This results in
all appends to a database getting serealised as the writes have to
update a data structure, that tracks the head of the database where
the next record has to be appended. Both the mutexes and the global
variable to update limit the concurrency during IBD.

The design of this database is motivated by in-memory MVCC databases
like Microsoft Hackathon, MemSQL, HyPer and NuoDB.

## Larger than memory

Most users of libbitcoin first try it on their smaller machines with
less RAM than required to hold all the blockchain data and indexes in
memory.

This database periodically runs garbage collection to move stale data
like spent transaction outputs and blocks with no UTXOs left to spend,
out to the disk.

The RAM then holds the most relevant data possible, which further
takes away the need for a cache in front of this database. After all,
this is an in memory database.

## Architecture - Key Decisions

The database management system will provide access to records using
Multi Version Concurrency Control. The ideal is to avoid depending on
mutexes for thread synchronization.

At a high level, the following design decisions motivate the rest of
the architecture. These key decisions are informed by the "the best
paper ever on in-memory multi version concurrency control", [Y. Wu, et
al., An Empirical Evaluation of In-Memory Multi-Version Concurrency
Control, in VLDB,
2017](https://15721.courses.cs.cmu.edu/spring2020/papers/03-mvcc1/wu-vldb2017.pdf)

1. Concurrency Control Protocol - Multi Version Timestamp Ordering to
   simplify first implementation.
1. Version Storage - Delta storage so we don't replicate bulky
   transaction data into older versions.
2. Garbage Collection - Background Vacuuming to move data from RAM to
   disk. Pointer swizzling to track the location of the records. As
   transactions are spent and blocks are confirmed they are moved to
   disk and indexes updated to point to data on disk instead of memory
   block and offset.
3. Index Management - Indexes records directly by primary key pointing
   to the master record on the main table, from where we can follow
   the delta storage, as required. For secondary index, maintain an
   indirection layer from secondary key to primary key, which can then
   be used to fetch the master record of the version chain.
4. Persistence - No write ahead or persistence log is used. Instead as
   bitcoin transactions and blocks are confirmed they are moved to
   disk by the garbage collector. This is because bitcoin is not an
   OLTP database, we don't need to manage a log for recovery
   purposes. In case of a crash, a bitcoin node obtains any lost data
   from other peers.
5. Variable length entities - Data like input/output scripts and
   witness programs are the only variable length data. We store them
   in special blocks that support variable length data. This means the
   scripts are not stored in the same memory location as the inputs
   and outputs. Those are stored in aligned memory for faster access
   and scripts are stored in separate memory blocks, but are memory
   aligned to allow fast access to start of the script.

### Multi Version Timestamp Ordering - MVTO

First writer wins. If a new writer can't update a record, then the
update operation returns an error. The blockchain layer will need to
retry the transaction. This might already be handled but needs to be
checked. The hunch is that the blockchain layer handles this
indirectly by depending on the sync protocol fetching the entire block
again.

This cost of retrying a block accept protocol seems expensive, but a
transaction rollback will happen very rarely and thus should be
amortised. Need to measure this once we have a working implementation.

### Delta Storage

Index points to master record which in turn points to a delta
record. For example, for block database, the delta record only has to
store the status field.

The master record always has the latest record, so index doesn't need
to be updated on each record update.

Garbage collector will delete the stale delta versions that no running or
future transactions will need.

### Garbage Collection

Periodically run garbage collector, or when a preset fraction of the
UTXO store is full.

Garbage collector will move spent UTXOs and blocks older than a preset
number of blocks to disk.

Depending on the memory available and configured to be used by the
various in memory tables, the database layer can be tweaked to work
for small laptops to big multi core servers. That is one of the goals.

### Index Management

Use [libcuckoo](https://github.com/efficient/libcuckoo) for providing
a fast, concurrent hash table as the index manager.

All indexes are limited to hash tables, so there is no support for
sequential scans over keys. This can be changed later to Bw-Tree to
support sequential scans.
