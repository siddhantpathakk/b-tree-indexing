#ifndef B_TREE_INDEX_H
#define B_TREE_INDEX_H

#include <math.h>
#include "types.h"

/**
 * Node functions
*/
// Creates empty Node (Works for both leaf and internal)
Node* createNode(int isLeaf);

//Set the global root node
Node* setGlobal(Node* root);

/**
 * B+ Tree index functions
*/
// Search for a key
Node* search(Node* cur, double_t key, int* internalNodesCtr);

//Recursively insert and return the pointer to the modified node
Node* insert(Node* cursor, double_t key, RecordPtr* record, Element* e);

//delete any keys less than or equals to FG_PCT_home
int deleteQuery(double_t FG_PCT_home, Node *node, Node **rootNode);
/*
    FG_PCT_home: value to identify keys less than or equal to that value to be deleted
    node: your root node 
    rootNode: your root node to be referenced and updated from deletion query

    returns int but return value is for recursive use, should not be used in main code.
*/

// Used to fix the pointers connecting leaf nodes after deleting
void fixLeaves(Node* root);

/**
 * Debugging and visualisation
*/
//Print Tree Structure for debugging
void printTree(Node* cursor, int level);

//Print all leaves of Tree from left to right
void printLeaves(Node* root);

//counts number of nodes (both leaf and non-leaf nodes) in the tree and returns it
int countNumNodes(Node *root);

//counts number of levels in the tree and returns it
int countNumLevels(Node *root);

int countLeafNodes(Node *node);

#endif