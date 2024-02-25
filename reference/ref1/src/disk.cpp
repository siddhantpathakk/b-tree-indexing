#include "disk.h"
using namespace std;
Disk::Disk(size_t diskSize, size_t blockSize) : diskSize(diskSize), blockSize(blockSize), totalMemoryUsed(0)
{
    dataStorage = new unsigned char[diskSize]();

    maxRecordsPerBlock = floor(blockSize / sizeof(Record)); //number of records per block
    maxBlocksPerDisk = floor(diskSize / blockSize);         //number of blocks per disk
    
    currentBlockId = 0;
    currentRecordId = 0;
    
    cout << "Disk Capacity Chosen: " << diskSize / pow(2, 20) << " MB" << endl;
    cout << "Block Size: " << blockSize << " B" << endl;

}
/// Inserts a new record with the specified data into the disk.
/// @param GAME_DATE_EST The date of the game associated with the record.
/// @param TEAM_ID_HOME The home team's ID for the game.
/// @param PTS_home The points scored by the home team in the game.
/// @param FG_PCT_home The field goal percentage of the home team.
/// @param FT_PCT_home The free throw percentage of the home team.
/// @param FG3_PCT_home The three-point field goal percentage of the home team.
/// @param AST_home The number of assists by the home team.
/// @param REB_home The number of rebounds by the home team.
/// @param HOME_TEAM_WINS Indicates whether the home team won the game.
/// @return A pointer to the newly inserted record on the disk, or nullptr if no memory is available.
Record *Disk::insertRecord(const string &GAME_DATE_EST, int TEAM_ID_HOME, int PTS_home, float FG_PCT_home, float FT_PCT_home, float FG3_PCT_home, int AST_home, int REB_home, int HOME_TEAM_WINS)
{
    if (currentBlockId >= maxBlocksPerDisk)
    {
        std::cout << "Error: No memory left to allocate new block";
        return nullptr;
    }

    Record *newRecord = getRecord(currentBlockId, currentRecordId);

    strncpy(newRecord->GAME_DATE_EST, GAME_DATE_EST.c_str(), sizeof(newRecord->GAME_DATE_EST));
    newRecord->TEAM_ID_HOME = TEAM_ID_HOME;
    newRecord->PTS_home = PTS_home;
    newRecord->FG_PCT_home = FG_PCT_home;
    newRecord->FT_PCT_home = FT_PCT_home;
    newRecord->FG3_PCT_home = FG3_PCT_home;
    newRecord->AST_home = AST_home;
    newRecord->REB_home = REB_home;
    newRecord->HOME_TEAM_WINS = HOME_TEAM_WINS;

    // Update memory usage and current record ID
    totalMemoryUsed += (currentBlockId == 0 && currentRecordId == 0) ? blockSize : 0;
    currentRecordId++;

    if (currentRecordId == maxRecordsPerBlock)
    {
        currentBlockId++;
        totalMemoryUsed = totalMemoryUsed + blockSize;
        currentRecordId = 0;
    }

    return newRecord;
}

/// Retrieves a pointer to a record from the disk based on block index and record index.
/// @param blockIdx The index of the block containing the record.
/// @param recordIdx The index of the record within the block.
/// @return A pointer to the specified record on the disk.
Record *Disk::getRecord(size_t blockIdx, size_t recordIdx)
{
    // Calculate the byte offset for the specified record on the disk
    size_t offset = (blockIdx * blockSize) + (recordIdx * sizeof(Record));

    // Interpret the memory at the calculated offset as a Record pointer and return it
    return reinterpret_cast<Record *>(dataStorage + offset);
}

/// Retrieves the maximum number of records that can be stored in a single block.
/// @return The maximum number of records per block.
size_t Disk::getRecordsPerBlock()
{
    return maxRecordsPerBlock;
}

/// Retrieves the number of blocks currently used on the disk.
/// @return The number of blocks used.
size_t Disk::getNumOfBlocksUsed()
{
    // The current block ID is 0-based, so adding 1 gives the total number of blocks used.
    return currentBlockId + 1;
}

/// Retrieves the block index of a record based on its memory address.
/// @param record A pointer to the record for which to find the block index.
/// @return The block index of the specified record.
size_t Disk::getBlockId(Record *record)
{
    // Calculate the block index by subtracting the base dataStorage address and dividing by blockSize.
    return (reinterpret_cast<unsigned char *>(record) - dataStorage) / blockSize;
}
