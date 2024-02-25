#include "BPTree.h"
#include <algorithm>
using namespace std;

void BPTree::insertKey(float key, Record *recordPtr)
{
    // Case 1: if root is null (no tree), create a new tree and set the key as the root
    if (this->root == nullptr)
    {
        createBPTree(key, recordPtr);
        return;
    }
    // Case 2: Duplicate: simply check if it exists
    if (auto keyRecords = this->searchRecord(key))
    {
        keyRecords->push_back(recordPtr);
        return;
    }

    vector<Node *> parents(1, nullptr);
    Node *currNode = this->root;
    int index = 0;

    // find the leaf to insert in and keep track of visited nodes in parents
    while (!currNode->isLeaf)
    {
        auto keysBegin = currNode->keys.begin();
        auto keysEnd = currNode->keys.end();
        index = upper_bound(keysBegin, keysEnd, key) - keysBegin; // Calculate the index for insertion
        parents.push_back(currNode);                              // Push the current node onto the parents stack
        currNode = currNode->pointers.at(index);                  // Update 'currNode' to the child node at the determined index
    }

    //  Case 3a: Root exists already, leaf node < max keys
    this->simpleLeafInsertion(currNode, key, recordPtr);

    // Case 3b: Leaf node keys >= max keys
    if (currNode->keys.size() > static_cast<unsigned long>(this->maxNumOfKeys))
    {
        Node *newNode = this->splitLeafNode(currNode);
        Node *parent = parents.back(); // last node in parents is the current parent
        parents.pop_back();
        key = newNode->keys.front(); // first key in the newNode

        // iteratively restructure the tree if the parent node is full
        while (parent != nullptr && parent->keys.size() == static_cast<unsigned long>(this->maxNumOfKeys))
            this->updateParent(parent, key, currNode, newNode);

        // Case 3b.1: inserted key become the root (B+ Tree level increase)
        if (parent == nullptr)
        {
            this->handleNullParent(parent, newNode, currNode, key);
            return;
        }
        // Case 3b.2: parent node has space to accomodate the inserted key index
        this->simpleParentInsertion(index, parent, newNode, key);
    }
}

/// Creates a new B+ Tree with a single leaf node containing the initial key and record.
/// @param key: The key to insert as the initial data.
/// @param recordPtr: A pointer to the associated record.
void BPTree::createBPTree(float key, Record *recordPtr)
{
    this->root = new Node(true); // create a new node in main memory and set it as root
    this->root->nextLeaf = nullptr;
    this->root->keys.push_back(key);
    this->root->records.push_back(vector<Record *>(1, recordPtr));
    this->numOfNodes++;
    this->levels++;
}

/// Inserts a key and associated record into a leaf node in a B+ Tree.
/// @param currNode: The current leaf node where the insertion will take place.
/// @param key: The key to insert.
/// @param recordPtr: A pointer to the associated record.
void BPTree::simpleLeafInsertion(Node *currNode, float key, Record *recordPtr)
{
    // Find the position where the 'key' should be inserted in the 'currNode->keys' vector
    auto it = upper_bound(currNode->keys.begin(), currNode->keys.end(), key);
    // Calculate the index for insertion by subtracting the iterator from the beginning
    int index = it - currNode->keys.begin();
    // Insert 'key' at the determined index to maintain sorted order
    currNode->keys.insert(currNode->keys.begin() + index, key);
    // Insert 'recordPtr' into the 'currNode->records' vector at the same index as 'key'
    currNode->records.insert(currNode->records.begin() + index, vector<Record *>(1, recordPtr));
}

/// Inserts a key and associated newParentNode as a child into a parent node.
/// @param index: The index at which 'key' should be inserted.
/// @param parent: The parent node where the insertion will take place.
/// @param newParentNode: The new child node to insert.
/// @param key: The key to insert.
void BPTree::simpleParentInsertion(int index, Node *parent, Node *newParentNode, float key)
{
    // Find the index where 'key' should be inserted in the parent node's keys manually
    index = upper_bound(parent->keys.begin(), parent->keys.end(), key) - parent->keys.begin();
    // Insert 'key' into the parent node's keys at the determined index
    parent->keys.insert(parent->keys.begin() + index, key);
    // Insert 'newParentNode' as a child of the parent node at the same index as 'key'
    parent->pointers.insert(parent->pointers.begin() + index + 1, newParentNode);
}

/// Splits a leaf node into two leaf nodes and returns the new leaf node.
/// @param sourceNode: The leaf node to split.
/// @return: A pointer to the newly created leaf node.
Node *BPTree::splitLeafNode(Node *sourceNode)
{
    // Create a new leaf node (newLeafNode)
    Node *newLeafNode = new Node(true);

    int separator = (this->maxNumOfKeys + 1) / 2;

    // Use iterators to split keys and records
    auto keyIt = sourceNode->keys.begin();
    auto recordIt = sourceNode->records.begin();

    advance(keyIt, separator);
    advance(recordIt, separator);

    // Move keys and records from sourceNode to newNode
    newLeafNode->keys.assign(keyIt, sourceNode->keys.end());
    newLeafNode->records.assign(recordIt, sourceNode->records.end());

    // Erase moved keys and records in sourceNode
    sourceNode->keys.erase(keyIt, sourceNode->keys.end());
    sourceNode->records.erase(recordIt, sourceNode->records.end());

    // Re-link the leaf nodes to account for the new split node
    newLeafNode->nextLeaf = sourceNode->nextLeaf;
    sourceNode->nextLeaf = newLeafNode;

    // Update the number of nodes in the B+ Tree
    this->numOfNodes++;

    return newLeafNode;
}

/// Splits an internal node (non-leaf node) into two internal nodes and returns the new internal node.
/// @param currNode: The internal node to split.
/// @param splitKey: A pointer to the key that separates the two internal nodes.
/// @return: A pointer to the newly created internal node.
Node *BPTree::splitParentNode(Node *currNode, float *splitKey)
{
    // Create a new internal node (newParentNode)
    Node *newParentNode = new Node(false);

    int separator = this->maxNumOfKeys / 2;

    // Use iterators to split keys and pointers
    auto keyIt = currNode->keys.begin();
    auto pointerIt = currNode->pointers.begin();

    // Advance iterators to split point (floor(N/2) keys/pointers)
    std::advance(keyIt, separator);
    std::advance(pointerIt, separator);

    // Move keys and pointers from currNode to newNode
    newParentNode->keys.assign(keyIt, currNode->keys.end());
    newParentNode->pointers.assign(pointerIt, currNode->pointers.end());

    // Remove keys and pointers from currNode
    currNode->keys.erase(keyIt, currNode->keys.end());
    currNode->pointers.erase(pointerIt, currNode->pointers.end());

    // Set the splitKey to the last key of currNode
    *splitKey = currNode->keys.back();
    currNode->keys.pop_back();

    // Maintain the linked structure among internal nodes
    newParentNode->pointers.insert(newParentNode->pointers.begin(), currNode->pointers.back());
    currNode->pointers.pop_back();

    // Update the number of nodes in the B+ Tree
    this->numOfNodes++;

    // Return a pointer to the newly created internal node (newParentNode)
    return newParentNode;
}

/// Updates the parent node by inserting 'key' and 'newNode' as children.
/// @param parent: The parent node to update.
/// @param key: The key to insert.
/// @param currNode: The current child node
/// @param newNode: The new child node to insert.
void BPTree::updateParent(Node *parent, float key, Node *currNode, Node *newNode)
{

    // Iterate up the tree, checking if the parent node is not NULL and has the maximum number of keys.
    // Find the index where 'key' should be inserted in the parent node's keys manually
    int index = 0;
    while (static_cast<unsigned long>(index) < parent->keys.size() && key > parent->keys[index])
        index++;

    // Insert 'key' into the parent node's keys at the determined index
    parent->keys.insert(parent->keys.begin() + index, key);

    // Insert 'newNode' as a child of the parent node at the same index as 'key'
    parent->pointers.insert(parent->pointers.begin() + index + 1, newNode);

    // Split the parent node and update 'newNode' and 'currNode' accordingly
    newNode = this->splitParentNode(parent, &key);
    currNode = parent;
}

/// Handles the case where the loop reaches a NULL parent (root node).
/// Creates a new root node and inserts 'key', 'currNode', and 'newNode' as children.
/// Updates the B+ Tree's root and levels.
/// @param parent: The parent node (which is NULL when root is reached).
/// @param newNode: The new child node to insert.
/// @param currNode: The current child node.
/// @param key: The key to insert.
void BPTree::handleNullParent(Node *parent, Node *newNode, Node *currNode, float key)
{
    // If the loop reaches a NULL parent, it means the root node has been reached.

    // Create a new root node
    parent = new Node(false);
    this->numOfNodes++;

    // Insert 'key' into the root node's keys
    parent->keys.push_back(key);

    // Insert 'currNode' and 'newNode' as children of the root node
    parent->pointers.push_back(currNode);
    parent->pointers.push_back(newNode);

    // Update the B+ Tree's root and levels
    this->root = parent;
    this->levels++;

    return;
}
