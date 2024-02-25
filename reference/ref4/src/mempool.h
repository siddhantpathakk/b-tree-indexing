#ifndef MEMPOOL_H
#define MEMPOOL_H

#include "types.h"

#include <vector>
#include <unordered_map>
#include <tuple>

using namespace std;

class MemoryPool
{
public:
  //constructor
  MemoryPool(size_t poolSize, size_t blockSize);

  //allocates a block from the memory pool
  bool allocateBlock();

  //allocates a chunk within a block
  Address allocate(size_t recordSize);

  //deletes existing record from its block, deallocates block if it becomes empty after deletion of record
  bool deallocate(Address recAddress, size_t sizeDeallocated);

  //returns data entry based on input parameters
  void *read(Address recAddress, size_t size);

  //write data entry into disk in a given memory address
  Address write(void *itemAddress, size_t size);

  // Method overloading
  //update data entry from disk from a given memory
  Address write(void *itemAddress, size_t size, Address diskAddress);

  size_t getPoolSize() const
  {
    return poolSize;
  }

  size_t getBlockSize() const
  {
    return blockSize;
  };

  size_t getBlockSizeUsed() const
  {
    return blockSizeUsed;
  };

  size_t getSizeUsed() const
  {
    return sizeUsed;
  }

  size_t getActualSizeUsed() const
  {
    return actualSizeUsed;
  }

  int getBlocksAllocated() const
  {
    return blocksAllocated;
  };

  int getBlocksAccessed() const
  {
    return blocksAccessed;
  }

  int resetBlocksAccessed()
  {
    int temp = blocksAccessed;
    blocksAccessed = 0;
    return temp;
  }

  //destructor
  ~MemoryPool();

private:
  size_t poolSize;    //max pool size
  size_t blockSize;      //fixed size of each block
  size_t sizeUsed;       //current storage size used based on number of blocks used
  size_t actualSizeUsed; //actual storage size used based on size of records
  size_t blockSizeUsed;  //storage size used within current block pointed to
  int blocksAllocated;
  int blocksAccessed;
  void *poolPtr;
  void *blockPtr;
};

#endif
