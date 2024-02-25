#include "BPTree.h"

// constructors for node
Node::Node(bool isLeaf)
{
    this->isLeaf = isLeaf;
}

Node::~Node() {}

// methods for node

// access next leaf node of the tree
Node *Node::getNextLeaf()
{
    return this->nextLeaf;
}

// obtain all keys stored within a node
std::vector<float> Node::getKeys()
{
    return this->keys;
}

// obtain all records stored in a particular node
std::vector<Record *> Node::getRecords(int idx)
{
    return this->records.at(idx);
}
// constructors for b+ tree
BPTree::BPTree(int blockSize)
{
    this->root = nullptr;
    this->blockSize = blockSize;
    this->maxNumOfKeys = (blockSize - sizeof(int *)) / (sizeof(int) + sizeof(int *));
    this->numOfNodes = 0;
    this->levels = 0;
    this->nodesSearched = 0;
}

BPTree::~BPTree() {}

// methods for b+ tree
Node *BPTree::getRoot()
{
    return this->root;
}

// creating the b+ tree (setting its root node)
void BPTree::setRoot(Node *r)
{
    this->root = r;
    return;
}

// maximum number of keys that the b+ tree can contain
int BPTree::getMaxNumOfKeys()
{
    return this->maxNumOfKeys;
}

// number of nodes currently contained within the b+ tree
int BPTree::getTotalNumOfNodes()
{
    return this->numOfNodes;
}

// number of levels of the b+ tree
int BPTree::getNumOfLevels()
{
    return this->levels;
}

// get block size of b+ tree
short BPTree::getBlockSize()
{
    return this->blockSize;
}

short BPTree::getNumOfNodesSearched()
{
    return this->nodesSearched;
}

void BPTree::setNumOfNodesSearched(int num)
{
    this->nodesSearched = num;
}

// Function to update the keys of the parent node
void BPTree::updateParentKeys(Node *current, Node *parent, int parentIndex, vector<Node *> &parents, vector<int> &indexes)
{
    while (parent != nullptr)
    {
        // Iteratively check and update the parent nodes
        if (parentIndex == 0)
        {
            // If parent's idx was 0 also, iteratively check its parent
            parent = parents.back();
            parents.pop_back();
            parentIndex = indexes.back();
            indexes.pop_back();
        }
        else
        {
            // If parent's idx was not 0, update the key with the new left most key of the leaf node
            int key = current->keys.front();
            parent->keys[parentIndex - 1] = key;
            break;
        }
    }
}

// Function to find the largest key in the B+ tree
float BPTree::findLargestKey()
{
    Node *currentNode = root;

    // Traverse to the rightmost leaf node, which will contain the largest key
    while (!currentNode->isLeaf)
        currentNode = currentNode->pointers.back();

    // The largest key will be the last key in the rightmost leaf node
    return currentNode->keys.back();
}
