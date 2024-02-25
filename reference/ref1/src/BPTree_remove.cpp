#include "BPTree.h"
#include <algorithm>
using namespace std;

/// @brief Removes the keys in ascending order up till the specified key.
/// @param key
void BPTree::remove(float key)
{
    while (true)
    {
        if (this->root == nullptr)
        {
            cout << "The tree is empty/null!" << endl;
            return;
        }

        Node *current = this->root;
        vector<Node *> parents(1, nullptr); // Stores the path of parent nodes
        vector<int> indexes;                // Stores the indexes used in the path
        int index;
        int minimumKeys = (this->maxNumOfKeys + 1) / 2;

        while (!current->isLeaf) // Save the internal nodes visited and the indexes used for the leaf node of the key
        {
            index = upper_bound(current->keys.begin(), current->keys.end(), 0) - current->keys.begin();
            parents.push_back(current);
            indexes.push_back(index);
            current = current->pointers.at(index);
        }

        // Find the key's index in the leaf node and delete the key and its records
        index = lower_bound(current->keys.begin(), current->keys.end(), 0) - current->keys.begin();
        if (current->keys[0] > key)
        {
            // cout << "The key is not found in the tree!" << endl;
            break;
        }
        // cout << "Key Deleted: " << current->keys[index] << endl;
        current->keys.erase(current->keys.begin() + index);
        current->records.erase(current->records.begin() + index);

        if (current == this->root && current->keys.size() == 0)
            // cout << "The root node is empty, setting root to null!" << endl;
            this->setRoot(nullptr);

        if (current->keys.size() >= static_cast<unsigned long>(minimumKeys))
        {
            if (index != 0)
                // cout << "Case: The number of keys is greater than the minimum number of keys required. So we can just proceed without merging or borrowing from neighbour nodes. " << endl;
                continue;

            while (parents.back() != nullptr)
            {
                if (indexes.back() == 0)
                {
                    parents.pop_back();
                    indexes.pop_back();
                }
                else
                {
                    key = current->keys.front();
                    current = parents.back();
                    index = indexes.back();
                    current->keys[index - 1] = key;
                    break;
                }
            }
        }
        else
        {
            int parentIndex = indexes.back();
            indexes.pop_back();
            Node *leftNode = nullptr;
            Node *rightNode = nullptr;
            Node *parent = parents.back();
            parents.pop_back();

            if (parentIndex > 0)
            {
                leftNode = parent->pointers.at(parentIndex - 1);
                if (leftNode->keys.size() > static_cast<unsigned long>(minimumKeys))
                {
                    this->borrowFromLeftNode(current, leftNode, parent, parentIndex);
                    continue;
                }
            }
            if (static_cast<unsigned long>(parentIndex) < parent->keys.size() - 1)
            {
                rightNode = parent->pointers.at(parentIndex + 1);
                if (rightNode->keys.size() > static_cast<unsigned long>(minimumKeys))
                {
                    this->borrowFromRightNode(current, rightNode, parent, parentIndex);
                    if (!index)
                        this->updateParentKeys(current, parent, parentIndex, parents, indexes);
                    continue;
                }
            }

            if (leftNode != nullptr)
            {
                this->mergeWithLeftNode(current, leftNode); // merge
                this->removeInternal(parent->keys.at(parentIndex - 2), parent, current);
            }
            else
            {
                this->mergeWithRightNode(current, rightNode);
                this->removeInternal(parent->keys.at(parentIndex), parent, rightNode);
            }
        }
    }
}

/// @brief Case 2: Borrow a key from the left node since it has more than the minimum number of keys.
/// @param current The current node.
/// @param leftNode The left neighbor node.
/// @param parent The parent node.
/// @param parentIndex The index of the parent node.
void BPTree::borrowFromLeftNode(Node *current, Node *leftNode, Node *parent, int parentIndex)
{
    // cout << "Case: Borrow a key from the left node since it has more than the minimum number of keys" << endl;
    current->keys.insert(current->keys.begin(), leftNode->keys.back());
    current->records.insert(current->records.begin(), leftNode->records.back());
    leftNode->keys.pop_back();
    leftNode->records.pop_back();
    parent->keys[parentIndex] = current->keys.front();
}

/// @brief Case 2: Unable to borrow from the left node. Borrow from the right node instead since it has more than the minimum number of keys.
/// @param current The current node.
/// @param rightNode The right neighbor node.
/// @param parent The parent node.
/// @param parentIndex The index of the parent node.
void BPTree::borrowFromRightNode(Node *current, Node *rightNode, Node *parent, int parentIndex)
{
    // cout << "Case: Unable to borrow from left node. Borrow from right Node instead since it has more than the minimum number of keys" << endl;
    current->keys.insert(current->keys.end(), rightNode->keys.front());
    current->records.insert(current->records.end(), rightNode->records.front());
    rightNode->keys.erase(rightNode->keys.begin());
    rightNode->records.erase(rightNode->records.begin());
    parent->keys[parentIndex] = rightNode->keys.front();
}

/// @brief Case 3: Merge with the left node as it doesn't have more than the minimum number of keys.
/// @param current The current node.
/// @param leftNode The left neighbor node.
void BPTree::mergeWithLeftNode(Node *current, Node *leftNode)
{
    // cout << "Case: Merge with Left Node as it doesn't have more than the minimum number of keys" << endl;
    while (current->keys.size() != 0)
    {
        leftNode->keys.push_back(current->keys.front());
        leftNode->records.push_back(current->records.front());
        current->keys.erase(current->keys.begin());
        current->records.erase(current->records.begin());
    }

    leftNode->nextLeaf = current->nextLeaf;
}

/// @brief Case 3: Merge with the right node as it doesn't have more than the minimum number of keys.
/// @param current The current node.
/// @param rightNode The right neighbor node.
void BPTree::mergeWithRightNode(Node *current, Node *rightNode)
{
    // cout << "Case: Merge with Right Node as it doesn't have more than the minimum number of keys" << endl;
    while (rightNode->keys.size() != 0)
    {
        current->keys.push_back(rightNode->keys.front());
        current->records.push_back(rightNode->records.front());
        rightNode->keys.erase(rightNode->keys.begin());
        rightNode->records.erase(rightNode->records.begin());
    }
    current->nextLeaf = rightNode->nextLeaf;
}

/// @brief Removes an internal key and its associated pointer from the B+ tree.
/// @param key The key to remove.
/// @param parent The parent node.
/// @param nodeToDelete The node to delete.
void BPTree::removeInternal(float key, Node *parent, Node *nodeToDelete)
{
    // Check if the parent is the root and has only one key
    if (parent == this->root)
    {
        if (parent->keys.size() == 1)
        {
            // If the nodeToDelete is the left child, set the root to the right child
            parent->pointers.at(0) == nodeToDelete ? this->setRoot(parent->pointers.at(1)) : this->setRoot(parent->pointers.at(0));
            return; // Return since root has been updated
        }
    }
    // Delete the key from the parent node
    int index = lower_bound(parent->keys.begin(), parent->keys.end(), key) - parent->keys.begin();
    parent->keys.erase(parent->keys.begin() + index);

    // Find the index of the nodeToDelete in parent's pointers
    for (index = 0; index < parent->pointers.size(); index++)
    {
        if (parent->pointers[index] == nodeToDelete)
        {
            break;
        }
    }
    // Erase the nodeToDelete's pointer from parent's pointers
    parent->pointers.erase(parent->pointers.begin() + index);
    this->numOfNodes--; // Decrement the total number of nodes

    // Return if the parent has more than or equal to half of the maximum keys
    if (parent->keys.size() >= this->maxNumOfKeys / 2)
    {
        return;
    }

    // Find the parent's left and right neighbors
    Node *previousParent = this->checkIfParentExists(this->root, parent);
    if (previousParent == nullptr)
    {
        // The parent node is the root node, so no redistribution or merging is possible
        return;
    }

    // Find the index of the parent node in previousParent's pointers
    for (index = 0; index < previousParent->pointers.size(); index++)
    {
        if (previousParent->pointers.at(index) == parent)
        {
            break;
        }
    }
    Node *leftNode;
    Node *rightNode;

    // If there is a left neighbor
    if (index > 0)
    {
        leftNode = parent->pointers.at(index - 1);

        // If the left neighbor has more than half of the maximum keys
        if (leftNode->keys.size() > this->maxNumOfKeys / 2)
        {
            // Move a key from the left neighbor to the parent node
            parent->keys.insert(parent->keys.begin(), previousParent->keys.at(index - 1));
            previousParent->keys[index - 1] = leftNode->keys.back();

            // Move a pointer from the left neighbor to the parent node
            parent->pointers.insert(parent->pointers.begin(), leftNode->pointers.back());

            // Remove the moved key and pointer from the left neighbor
            leftNode->keys.pop_back();
            leftNode->pointers.pop_back();
        }
    }

    // If there is a right neighbor
    if (index < previousParent->pointers.size() - 1)
    {
        rightNode = previousParent->pointers.at(index + 1);

        // If the right neighbor has more than half of the maximum keys
        if (rightNode->keys.size() > this->maxNumOfKeys / 2)
        {
            // Move a key from the right neighbor to the parent node
            parent->keys.push_back(previousParent->keys.at(index));
            parent->keys[index] = rightNode->keys.front();
            rightNode->keys.erase(rightNode->keys.begin());

            // Move a pointer from the right neighbor to the parent node
            parent->pointers.push_back(rightNode->pointers.front());
            rightNode->pointers.erase(rightNode->pointers.begin());

            return; // Redistribution is successful
        }
    }

    // Redistribution is not possible, so merge with left or right neighbor
    if (index > 0)
    {
        leftNode->keys.push_back(previousParent->keys.at(index - 1));

        // Move all keys from the parent to the left neighbor
        while (parent->keys.size() != 0)
        {
            leftNode->keys.push_back(parent->keys.front());
        }

        // Move all pointers from the parent to the left neighbor
        while (parent->pointers.size() != 0)
        {
            leftNode->pointers.push_back(parent->pointers.front());
        }

        // Recursively remove the parent's key from its parent
        this->removeInternal(previousParent->keys.at(index - 1), previousParent, parent);
    }
    else if (index < previousParent->pointers.size() - 1)
    {
        parent->keys.push_back(previousParent->keys.at(index));

        // Move all keys from the right neighbor to the parent
        while (rightNode->keys.size() != 0)
        {
            parent->keys.push_back(rightNode->keys.front());
        }

        // Move all pointers from the right neighbor to the parent
        while (rightNode->pointers.size() != 0)
        {
            parent->pointers.push_back(rightNode->pointers.front());
        }

        // Recursively remove the parent's key from its parent
        this->removeInternal(previousParent->keys.at(index), previousParent, rightNode);
    }
}

/// @brief Checks if a parent node exists and is the parent of a child node.
/// @param parent The potential parent node.
/// @param child The potential child node.
/// @return The parent node if found, otherwise nullptr.
Node *BPTree::checkIfParentExists(Node *parent, Node *child)
{
    int key = 0;
    int index = 0;
    Node *current = child;

    // Find the key of the leftmost leaf node (child)
    while (current->isLeaf == false)
        current = current->pointers.front();
    key = current->keys.front();

    // Traverse upwards through the parent nodes to find the parent of the child
    while (parent->isLeaf == false)
    {
        index = upper_bound(parent->keys.begin(), parent->keys.end(), key) - parent->keys.begin();
        if (parent->pointers.at(index) == child)
            return parent; // Return the parent node if found
        else
            parent = parent->pointers.at(index);
    }

    return nullptr; // Return nullptr if the parent node is not found
}
