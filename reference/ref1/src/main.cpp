#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <unordered_set>
#include "disk.h"
#include "BPTree.h"
#include "utils.h"

using namespace std;
using namespace chrono;

int main()
{
    // Set block size to 400B and disk size to 500MB
    int BLOCKSIZE = 400;
    int DISKSIZE = 500 * pow(2, 20);

    Disk disk(DISKSIZE, BLOCKSIZE);
    BPTree tree(BLOCKSIZE);

    /*
    =============================================================
    Experiment 1:
    store the data (which is about NBA games and described in Part 4) on the disk (as specified in Part 1) and report:
    • the number of records;
    • the size of a record;
    • the number of records stored in a block;
    • the number of blocks for storing the data;
    =============================================================
    */

    cout << "\nExperiment 1:" << endl;
    // opening of data file
    ifstream file("data/sorted_data.txt");

    Record *newRecord = nullptr;
    string line;
    int numRecords = 0;

    // parsing of data
    getline(file, line);
    while (getline(file, line))
    {
        try
        {
            istringstream lineStream(line);
            string GAME_DATE_EST, TEAM_ID_HOME, PTS_home, FG_PCT_home, FT_PCT_home, FG3_PCT_home, AST_home, REB_home, HOME_TEAM_WINS;
            getline(lineStream, GAME_DATE_EST, '\t');
            getline(lineStream, TEAM_ID_HOME, '\t');
            getline(lineStream, PTS_home, '\t');
            getline(lineStream, FG_PCT_home, '\t');
            getline(lineStream, FT_PCT_home, '\t');
            getline(lineStream, FG3_PCT_home, '\t');
            getline(lineStream, AST_home, '\t');
            getline(lineStream, REB_home, '\t');
            getline(lineStream, HOME_TEAM_WINS, '\t');

            newRecord = disk.insertRecord(GAME_DATE_EST, stoi(TEAM_ID_HOME), stoi(PTS_home), stod(FG_PCT_home), stod(FT_PCT_home), stod(FG3_PCT_home), stoi(AST_home), stoi(REB_home), stoi(HOME_TEAM_WINS));
            tree.insertKey(newRecord->FG_PCT_home, newRecord);

            numRecords++;
        }
        catch (const exception &e)
        {
            cerr << "An exception occurred: " << e.what() << endl;
        }
    }

    // delete newRecord;
    file.close();

    // experiment 1 log
    cout << "==================== Experiment 1 ====================" << endl;
    cout << "Number of records stored: " << numRecords << endl;
    cout << "Size of a stored record: " << sizeof(Record) << " bytes" << endl;
    cout << "Number of records stored in a block: " << disk.getRecordsPerBlock() << endl;
    cout << "Number of blocks created for storing the data: " << disk.getNumOfBlocksUsed() << endl;
    cout << "================ End of Experiment 1 ================" << endl;
    cout << "" << endl;

    /*
    =============================================================
    Experiment 2: build a B+ tree on the attribute "FG_PCT_home" by inserting the records sequentially and report the following statistics:
    the parameter n of the B+ tree;
    =============================================================
    */

    // experiment 2 log
    cout << "==================== Experiment 2 ====================" << endl;
    cout << "n = " << tree.getMaxNumOfKeys() << endl;
    cout << "Total number of nodes in the tree = " << tree.getTotalNumOfNodes() << endl;
    cout << "Total number of levels of the tree = " << tree.getNumOfLevels() << endl;
    cout << "Keys in the root node: ";
    tree.printKeys(tree.getRoot());
    cout << "================ End of Experiment 2 ================" << endl;
    cout << "" << endl;

    /*
    =============================================================
    Experiment 3:
    retrieve those games with the “FG_PCT_home” equal to 0.5 and report the following statistics:
    • the number of index nodes the process accesses;
    • the number of data blocks the process accesses;
    • the average of “FG3_PCT_home” of the records that are returned;
    • the running time of the retrieval process (please specify the method you use for measuring the running time of a piece of code)
    • the number of data blocks that would be accessed by a brute-force linear scan method
        (i.e., it scans the data blocks one by one) and its running time (for comparison)
    =============================================================
    */
    // Initialize variables
    tree.setNumOfNodesSearched(0);
    auto startTime = high_resolution_clock::now();

    // Search for records with FG_PCT_home equal to 0.5
    vector<Record *> *matchingRecords = tree.searchRecord(0.5);
    size_t numMatchingRecords = matchingRecords->size();

    auto endTime = high_resolution_clock::now();
    auto elapsedTime = duration_cast<microseconds>(endTime - startTime);

    // Calculate statistics for matching records
    unordered_set<size_t> uniqueBlockIds;
    float totalFG3Pct = 0;
    for (Record *record : *matchingRecords)
    {
        uniqueBlockIds.insert(disk.getBlockId(record));
        totalFG3Pct += record->FG3_PCT_home;
    }

    int bruteTime;
    int linearScanBlocksAccessed = 0;
    tie(linearScanBlocksAccessed, bruteTime) = exp3LinearScan(disk, linearScanBlocksAccessed);

    // Experiment 3 log
    cout << "==================== Experiment 3 ====================" << endl;
    cout << "No. of Index Nodes Process Accesses: " << tree.getNumOfNodesSearched() << endl;
    cout << "No. of Data Blocks Process Accesses: " << uniqueBlockIds.size() << endl;
    cout << "Average of “FG3_PCT_home” of the Records Returned: " << totalFG3Pct / numMatchingRecords << endl;
    cout << "Running time of Retrieval Process (microseconds): " << elapsedTime.count() << "µs" << endl;
    cout << "Data Blocks Accessed by linear scan: " << linearScanBlocksAccessed << endl;
    cout << "Linear Scan retrieval time (in microseconds): " << bruteTime << "µs" << endl;
    cout << "================ End of Experiment 3 ================" << endl;
    cout << "" << endl;
    /*
    =============================================================
    Experiment 4:
    retrieve those movies with the attribute “FG_PCT_home” from 0.6 to 1, both inclusively and report the following statistics
    • the number of index nodes the process accesses;
    • the number of data blocks the process accesses;
    • the average of “FG3_PCT_home” of the records that are returned;
    • the running time of the retrieval process;
    • the number of data blocks that would be accessed by a brute-force linear scan method
        (i.e., it scans the data blocks one by one) and its running time (for comparison)
    =============================================================
    */

    vector<Record *> result;
    vector<float> keys;
    float lower = 0.6;
    float upper = 1.0;
    int lowerIdx, upperIdx, leafNodesSearched = 0;
    bool searching = true;
    tree.setNumOfNodesSearched(0);

    auto pre = high_resolution_clock::now();
    Node *resultNode = tree.searchNode(lower);

    // locating of first record where key >= lower
    while (searching)
    {
        keys = resultNode->getKeys();
        lowerIdx = lower_bound(keys.begin(), keys.end(), lower) - keys.begin();
        upperIdx = lower_bound(keys.begin(), keys.end(), upper) - keys.begin();

        for (int i = lowerIdx; i < upperIdx; i++)
        {
            for (Record *rec : resultNode->getRecords(i))
                result.push_back(rec);
        }

        upperIdx = static_cast<unsigned long>(upperIdx) == keys.size() ? upperIdx - 1 : upperIdx;
        if (keys.at(upperIdx) >= upper || keys.at(upperIdx) >= tree.findLargestKey())
            searching = false;

        else
        {
            resultNode = resultNode->getNextLeaf();
            leafNodesSearched++;
        }
    }

    auto post = high_resolution_clock::now();
    auto timeTaken = duration_cast<microseconds>(post - pre);

    // finding the FG3_PCT_home values of all records where 0.6 <= FG_PCT_home <= 1.0
    unordered_set<size_t> resultSet;
    float totalFG3 = 0;
    for (Record *rec : result)
    {
        resultSet.insert(disk.getBlockId(rec));
        totalFG3 += rec->FG3_PCT_home;
    }
    linearScanBlocksAccessed = 0;
    tie(linearScanBlocksAccessed, bruteTime) = exp4LinearScan(disk, 0.6, 1.0, linearScanBlocksAccessed);

    // experiment 4 log
    cout << "==================== Experiment 4 ====================" << endl;
    cout << "No of index blocks searched: " << tree.getNumOfNodesSearched() + leafNodesSearched << endl;
    cout << "No of data blocks searched: " << resultSet.size() << endl;
    cout << "Average of FG3_PCT_home: " << totalFG3 / result.size() << endl;
    cout << "B+ Tree retrieval time (in microseconds): " << timeTaken.count() << "μs" << endl;
    cout << "Data blocks accessed by linear scan: " << linearScanBlocksAccessed << endl;
    cout << "Linear Scan retrieval time (in microseconds): " << bruteTime << "μs" << endl;
    cout << "================ End of Experiment 4 ================" << endl;
    cout << "" << endl;

    /*
    =============================================================
    Experiment 5: delete those movies with the attribute “FG_PCT_home” below 0.35 inclusively, update the B+ tree accordingly, and report the following statistics:
    • the number nodes of the updated B+ tree;
    • the number of levels of the updated B+ tree;
    • the content of the root node of the updated B+ tree(only the keys);
    • the running time of the process;
    • the number of data blocks that would be accessed by a brute-force
    linear scan method (i.e., it scans the data blocks one by one) and its running time (for comparison)
    =============================================================
    */
    linearScanBlocksAccessed = 0;
    tie(linearScanBlocksAccessed, bruteTime) = exp5LinearScan(disk, 0.35, linearScanBlocksAccessed);
    pre = high_resolution_clock::now();
    tree.remove(0.35);
    post = high_resolution_clock::now();
    timeTaken = duration_cast<microseconds>(post - pre);

    // experiment 5 log
    cout << "==================== Experiment 5 ====================" << endl;
    cout << "No of nodes after deletion: " << tree.getTotalNumOfNodes() << endl;
    cout << "No of levels after deletion: " << tree.getNumOfLevels() << endl;
    cout << "Root node after deletion: ";
    tree.printKeys(tree.getRoot());
    cout << "Running time for deletion process (in microseconds): " << timeTaken.count() << "μs" << endl;
    cout << "Data blocks accessed by linear scan: " << linearScanBlocksAccessed << endl;
    cout << "Running Time of the Brute-Force Linear Scan Method: " << bruteTime << "μs" << endl;
    cout << "================ End of Experiment 5 ================" << endl;
    cout << "" << endl;

    }