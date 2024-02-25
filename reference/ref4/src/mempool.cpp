#include "mempool.h"
#include "types.h"

#include <iostream>
#include <vector>
#include <tuple>
#include <cstring>

using namespace std;

//constructor
MemoryPool::MemoryPool(size_t poolSize, size_t blockSize)
{
  this->poolSize = poolSize; //capacity of memory pool
  this->blockSize = blockSize; //capacity of each block
  this->sizeUsed = 0;
  this->actualSizeUsed = 0;
  this->blocksAllocated = 0;

  // Create pool of blocks.
  this->poolPtr = operator new(poolSize);
  memset(poolPtr, '\0', poolSize); // initializing pool pointers to null
  this->blockPtr = nullptr;
  this->blockSizeUsed = 0;
  this->blocksAccessed = 0;
}

bool MemoryPool::allocateBlock()
{
  if (sizeUsed + blockSize <= poolSize)
  {
    sizeUsed += blockSize;
    blockPtr = (char *)poolPtr + blocksAllocated * blockSize;
    blockSizeUsed = 0; //reset offset to 0
    blocksAllocated += 1;
    return true;
  }
  else
  {
    cout << "No memory to allocate new block! [" << sizeUsed << "out of" << maxPoolSize << " is currently in use.]" << '\n';
    return false;
  }
}

Address MemoryPool::allocate(size_t recordSize)
{
  if (recordSize > blockSize)
  {
    cout << "Size required is larger than block size! (Size required:" << recordSize << "> Block size:" << blockSize << ")" << '\n';
    throw invalid_argument("Size requested is too large!");
  }

  if (blocksAllocated == 0 || (blockSizeUsed+recordSize > blockSize))
  {
    if (allocateBlock() == false)
    {
      throw logic_error("Block allocation failed!");
    }
  }

  short int blockOffset = blockSizeUsed;

  blockSizeUsed += recordSize;
  actualSizeUsed += recordSize;

  Address recAddress = {blockPtr, blockOffset};

  return recAddress;
}

bool MemoryPool::deallocate(Address recAddress, size_t sizeDeallocated)
{
  try
  {
    //deleting the record from the block
    void *addressToDelete = (char *)recAddress.blockAddress + recAddress.blockOffset;
    memset(addressToDelete, '\0', sizeDeallocated);

    actualSizeUsed -= sizeDeallocated;

    //case where block becomes empty after deallocation of record
    //creating a test block full of NULL values to test whether actual block is empty
    unsigned char testBlock[blockSize];
    memset(testBlock, '\0', blockSize);

    if (memcmp(testBlock, recAddress.blockAddress, blockSize) == 0) //if the first blockSize bytes of memory are equal in testBlock and in blockAddress
    {
      sizeUsed -= blockSize;
      blocksAllocated--;
    }

    return true;
  }
  catch (...) //for all kinds of exceptions caught
  {
    cout << "Could not remove record/block at given address (" << recAddress.blockAddress << ") and offset (" << recAddress.blockOffset << "). Please try again" << '\n';
    return false;
  };
}

void *MemoryPool::read(Address recAddress, size_t size)
{
  void *mainMemAddress = operator new(size);
  memcpy(mainMemAddress, (char *)recAddress.blockAddress + recAddress.blockOffset, size);

  blocksAccessed++;

  return mainMemAddress;
}

Address MemoryPool::write(void *itemAddress, size_t size)
{
  Address diskAddress = allocate(size);
  memcpy((char *)diskAddress.blockAddress + diskAddress.blockOffset, itemAddress, size);

  blocksAccessed++;

  return diskAddress;
}

//method overloading
Address MemoryPool::write(void *itemAddress, size_t size, Address diskAddress)
{
  memcpy((char *)diskAddress.blockAddress + diskAddress.blockOffset, itemAddress, size);

  blocksAccessed++;

  return diskAddress;
}

//deconstructor
MemoryPool::~MemoryPool(){};
