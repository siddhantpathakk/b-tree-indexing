#ifndef BPTREE_H
#define BPTREE_H

#include <vector>
#include "disk.h"

using namespace std;
// define a node in the b+ tree
class Node
{
private:
    bool isLeaf; // whether this node is a leaf node
    Node *nextLeaf;
    vector<Node *> pointers; // a pointer to an array of nodes in this disk
    vector<float> keys;      // a pointer to an array of keys in this node
    vector<vector<Record *>> records;

    int numOfKeys; // current number of keys in this node

public:
    // constructors
    Node(bool isLeaf);
    ~Node();

    Node *getNextLeaf();
    vector<float> getKeys();
    vector<Record *> getRecords(int index);

    friend class BPTree;
};

// the b+ tree itself
class BPTree
{
private:
    Node *root;          // pointer to root if it's loaded
    int maxNumOfKeys;    // maximum number of keys in a node
    short nodesSearched; // number of nodes that is accessed when searching
    int numOfNodes;      // number of nodes in B+ tree
    short blockSize;     // size of block == size of node
    int levels;

public:
    // constructors
    BPTree(int blockSize);
    ~BPTree();

    // methods
    Node *getRoot();
    void setRoot(Node *node);

    int getMaxNumOfKeys();
    int getTotalNumOfNodes();
    int getNumOfLevels();
    short getBlockSize();

    short getNumOfNodesSearched();
    void setNumOfNodesSearched(int num);

    void printKeys(Node *node);
    void printTree(Node *node);

    float findLargestKey();
    // search
    Node *searchNode(float key);
    vector<Record *> *searchRecord(float key);
    void gotoleaf(Node *node, float key, int keyIndex);
    int findKeyIndexInNode(Node *node, float key);
    int findChildIndex(Node *node, float key);
    // insert
    void insertKey(float key, Record *recordPtr);
    void createBPTree(float key, Record *recordPtr);
    void simpleLeafInsertion(Node *currNode, float key, Record *recordPtr);
    void simpleParentInsertion(int index, Node *parent, Node *newNode, float key);
    Node *splitLeafNode(Node *sourceNode);
    Node *splitParentNode(Node *currNode, float *key);
    void updateParent(Node *parent, float key, Node *currNode, Node *newNode);
    void handleNullParent(Node *parent, Node *newNode, Node *currNode, float key);
    // delete
    void remove(float key);
    void borrowFromLeftNode(Node *current, Node *leftNode, Node *parent, int parentIndex);
    void borrowFromRightNode(Node *current, Node *rightNode, Node *parent, int parentIndex);
    void mergeWithLeftNode(Node *current, Node *leftNode);
    void mergeWithRightNode(Node *current, Node *rightNode);
    Node *checkIfParentExists(Node *parent, Node *child);
    void removeInternal(float key, Node *parent, Node *nodeToDelete);
    void updateParentKeys(Node *current, Node *parent, int parentIndex, vector<Node *> &parents, vector<int> &indexes);
};

#endif
