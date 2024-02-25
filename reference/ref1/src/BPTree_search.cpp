#include "BPTree.h"
#include <algorithm>

using namespace std;

Node *BPTree::searchNode(float key)
{
    if (this->root == nullptr) // check if the tree is empty
    {
        cout << "This Tree is null/empty!" << endl;
        return nullptr; // if empty, return null pointer
    }

    Node *currentNode = root; // point to root
    int keyIndex;
    this->nodesSearched++;

    while (!currentNode->isLeaf)
    {
        keyIndex = findChildIndex(currentNode, key);      // find index of child node
        currentNode = currentNode->pointers.at(keyIndex); // get the node
        this->nodesSearched++;
    }

    return currentNode;
}
/// Searches for records associated with a given key in the B+ tree.
/// \param key The key to search for.
/// \return A pointer to a vector of records matching the key, or nullptr if not found.
vector<Record *> *BPTree::searchRecord(float key)
{
    // Check if the tree is empty
    if (this->root == nullptr)
    {
        return nullptr;
    }

    Node *currentNode = root;
    int keyIndex;
    this->nodesSearched++;

    // Traverse the tree to reach a leaf node
    while (!currentNode->isLeaf)
    {
        keyIndex = findChildIndex(currentNode, key);
        currentNode = currentNode->pointers.at(keyIndex);
        this->nodesSearched++;
    }

    // Find the index of the key in the leaf node
    keyIndex = findKeyIndexInNode(currentNode, key);
    if (keyIndex >= 0)
        return &(currentNode->records.at(keyIndex));

    return nullptr;
}

/// Traverses down the tree to reach the leaf node containing a given key.
/// \param node The current node being processed.
/// \param key The key to reach in the leaf node.
/// \param keyIndex The index of the key in the current node.
void BPTree::gotoleaf(Node *node, float key, int keyIndex)
{
    while (!node->isLeaf)
    {
        keyIndex = findChildIndex(node, key);
        node = node->pointers.at(keyIndex);
        this->nodesSearched++;
    }
}

/// Finds the index of the child node that should contain the given key.
/// \param node The current node being processed.
/// \param key The key to find the child node for.
/// \return The index of the child node in the current node's pointers vector.
int BPTree::findChildIndex(Node *node, float key)
{
    // Find the index of the child node that contains the key
    auto keysBegin = node->keys.begin();
    auto keysEnd = node->keys.end();
    auto keyIterator = upper_bound(keysBegin, keysEnd, key);
    return (keyIterator - keysBegin);
}

/// Finds the index of the key within a leaf node using binary search.
/// \param node The leaf node being processed.
/// \param key The key to find in the node.
/// \return The index of the key in the node's keys vector, or -1 if not found.
int BPTree::findKeyIndexInNode(Node *node, float key)
{
    // Binary search for the key within the leaf node
    auto keysBegin = node->keys.begin();
    auto keysEnd = node->keys.end();
    auto index = lower_bound(keysBegin, keysEnd, key);

    if (index != keysEnd && *index == key)
        return index - keysBegin;

    return -1;
}
