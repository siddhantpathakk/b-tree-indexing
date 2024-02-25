#ifndef DISK_H
#define DISK_H

#include <cstddef>
#include <string>
#include <iostream>
#include <cmath>
#include <cstring>

using namespace std;

struct Record
{
    char GAME_DATE_EST[11];
    int TEAM_ID_HOME;
    int PTS_home;
    float FG_PCT_home;
    float FT_PCT_home;
    float FG3_PCT_home;
    int AST_home;
    int REB_home;
    int HOME_TEAM_WINS;
};

// Allocation of memory w methods & variables for handling records colletion & each individual record.
class Disk
{
private:
    size_t blockSize;
    size_t diskSize;
    size_t currentBlockId;
    size_t currentRecordId;
    unsigned char *dataStorage;

    size_t maxRecordsPerBlock;
    size_t maxBlocksPerDisk;
    size_t totalMemoryUsed;

public:
    // constructor
    Disk(size_t aDiskSize, size_t aBlockSize);

    // methods for records
    Record *insertRecord(const string &GAME_DATE_EST, int TEAM_ID_HOME, int PTS_home, float FG_PCT_home, float FT_PCT_home, float FG3_PCT_home, int AST_home, int REB_home, int HOME_TEAM_WINS);
    bool deleteRecord(size_t blockIdx, size_t recordIdx);
    Record *getRecord(size_t blockIdx, size_t recordIdx);

    // methods for blocks
    size_t getRecordsPerBlock();

    size_t getNumOfBlocksUsed();

    size_t getBlockId(Record *record);
};

#endif
