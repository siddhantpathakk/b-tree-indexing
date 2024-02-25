#include "disk.h"
#include "utils.h"

using namespace std;
using namespace chrono;

tuple<int, int> exp3LinearScan(Disk disk, int linearScanBlocksAccessed)
{
    Record *currentRecord;
    auto startTime = high_resolution_clock::now();
    int flag = 0;

    for (int blockIndex = 0; static_cast<unsigned long>(blockIndex) < disk.getNumOfBlocksUsed(); blockIndex++)
    {
        if (flag == 1)
        {
            linearScanBlocksAccessed++;
            flag = 0;
        }

        for (int recordIndex = 0; static_cast<unsigned long>(recordIndex) < disk.getRecordsPerBlock(); recordIndex++)
        {
            currentRecord = disk.getRecord(blockIndex, recordIndex);
            if (currentRecord->FG_PCT_home <= 0.5)
                flag = 1;
            else
            {
                auto endTime = high_resolution_clock::now();
                return make_tuple(linearScanBlocksAccessed, duration_cast<microseconds>(endTime - startTime).count());
            }
        }
    }
    auto endTime = high_resolution_clock::now();
    return make_tuple(linearScanBlocksAccessed, duration_cast<microseconds>(endTime - startTime).count());
}

tuple<int, int> exp4LinearScan(Disk disk, float lower, float upper, int linearScanBlocksAccessed)
{
    Record *currentRecord;
    auto startTime = high_resolution_clock::now();
    int flag = 0;
    for (int blockIndex = 0; static_cast<unsigned long>(blockIndex) < disk.getNumOfBlocksUsed(); blockIndex++)
    {
        linearScanBlocksAccessed++;

        for (int recordIndex = 0; static_cast<unsigned long>(recordIndex) < disk.getRecordsPerBlock(); recordIndex++)
        {
            currentRecord = disk.getRecord(blockIndex, recordIndex);
            if (currentRecord->FG_PCT_home >= lower && currentRecord->FG_PCT_home <= upper)
                continue;
        }
    }
    auto endTime = high_resolution_clock::now();
    return make_tuple(linearScanBlocksAccessed, duration_cast<microseconds>(endTime - startTime).count());
}

tuple<int, int> exp5LinearScan(Disk disk, float keysToDeleteBelow, int linearScanBlocksAccessed)
{
    Record *currentRecord;
    vector<float> selectedKeys;
    int flag = 0;
    auto startTime = high_resolution_clock::now();
    for (int blockIndex = 0; static_cast<unsigned long>(blockIndex) < disk.getNumOfBlocksUsed(); blockIndex++)
    {
        if (flag == 1)
        {
            linearScanBlocksAccessed++;
            flag = 0;
        }
        for (int recordIndex = 0; static_cast<unsigned long>(recordIndex) < disk.getRecordsPerBlock(); recordIndex++)
        {
            currentRecord = disk.getRecord(blockIndex, recordIndex);
            if (currentRecord->FG_PCT_home <= keysToDeleteBelow)
            {
                flag = 1;
                continue;
            }
            selectedKeys.push_back(currentRecord->FG_PCT_home);
        }
    }
    auto endTime = high_resolution_clock::now();
    return make_tuple(linearScanBlocksAccessed, duration_cast<microseconds>(endTime - startTime).count());
}