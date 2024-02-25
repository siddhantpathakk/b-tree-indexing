#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "types.h"
#include "block.h"

Node* ROOT=NULL;
//Set the global root node
Node* setGlobal(Node* root){
    ROOT=root;
}
// Creates empty Node (Works for both leaf and internal)
Node* createNode(int isLeaf) {
    Node* newNode = malloc(sizeof(Node));
    if (newNode != NULL) {
        newNode->isLeaf = isLeaf;
        newNode->numKeys = 0;
        for (int i = 0; i < MAX_KEYS; i++) {
            (&newNode->keys[i])->type = NOT_IN_USE;  // -1 indicates empty (not filled yet)
            newNode->children[i] = NULL;
        }
        newNode->children[MAX_KEYS] = NULL;
    }
    return newNode;
}

//Checks for presence of key and returns address of the block if found
Node* search(Node* cur, double_t key, int* internalNodesCtr){
    if(cur->isLeaf==1){
        int i=0;
        while(i<MAX_KEYS){
            if((&cur->keys[i])->value == key && (&cur->keys[i])->type != NOT_IN_USE){
                return cur;
            }
            i++;
        }
        return NULL;
    }

    int i=0;
    (*internalNodesCtr)++;
    while(i < MAX_KEYS && key >= (&cur->keys[i])->value && ((&cur->keys[i])->type != NOT_IN_USE)) { //Any key will be Implicity greater than -2
        i++;
    }

    while(i > 0 && (&cur->keys[i-1])->type == REPEATED){ //Last Non-null key found
        i--;
    }

    return search(cur->children[i],key, internalNodesCtr);
}

// Leaf node splitting
Node* _splitLeaf(Node* origNode, double_t key_value, RecordPtr* record, Element* e) {
    //Empty leaf
    Node* newNode = createNode(1);

    // We use temporary lists to store the keys and children pointers prior to shifting
    key tempKeys[MAX_KEYS + 1];
    void* tempPtrs[MAX_KEYS + 2];

    //Copy the array
    for (int i = 0; i < MAX_KEYS; i++) {
        tempKeys[i] = origNode->keys[i];
        tempPtrs[i] = origNode->children[i];
    }

    tempPtrs[MAX_KEYS] = origNode->children[MAX_KEYS];

    // Find index to insert key at
    int i = 0;
    while (i < MAX_KEYS && (&tempKeys[i])->value <= key_value) {
        i++;
    }

    // Shift keys and pointers above index to the right
    for (int j = MAX_KEYS; j > i; j--) {
        tempKeys[j] = tempKeys[j-1];
    }

    for (int j = MAX_KEYS + 1; j > i; j--) {
        tempPtrs[j] = tempPtrs[j-1];
    }

    // Insert new key and pointer into empty slot
    (&tempKeys[i])->value = key_value;
    (&tempKeys[i])->type = FIRST_OCCURENCE;
    tempPtrs[i] = record;

    int splitPt = ceil((MAX_KEYS+1)/2.0);
    //Distribute keys and pointers
    for(int i=splitPt, j=0; i<MAX_KEYS+1; i++, j++){
        newNode->keys[j] = tempKeys[i];
        newNode->children[j] = tempPtrs[i];
        (&origNode->keys[i])->type = NOT_IN_USE;
        origNode->children[i]=NULL;
    }

    origNode->children[MAX_KEYS] = tempPtrs[MAX_KEYS + 1];

    //Keep track of the number of key and ptr values
    origNode->numKeys = splitPt;
    newNode->numKeys = (MAX_KEYS+1)/2;

    for(int i=0; i<splitPt; i++){
        origNode->keys[i] = tempKeys[i];
        origNode->children[i] = tempPtrs[i];
    }

    // Update "next leaf node" pointers
    newNode->children[MAX_KEYS] = origNode->children[MAX_KEYS];
    origNode->children[MAX_KEYS] = (Node *) newNode;

    //Does the first key of newNode appear before?
    int internalNodesCtr = 0;
    if(search(ROOT,(&newNode->keys[0])->value, &internalNodesCtr)){
        int j=1;
        while(j<MAX_KEYS && (&newNode->keys[j])->type != NOT_IN_USE){
            if((&newNode->keys[j])->value != (&newNode->keys[j-1])->value){
                //New key found
                e->key = (&newNode->keys[j])->value;
                e->ptr = newNode;
                e->dupli = 0; //have a new key in block
                key prev;
                prev.value = -1;
                prev.type = NOT_IN_USE;
                e->prev = prev;
                return newNode;
            }
            j++;
        }
        //No new key
        e->key = (&newNode->keys[0])->value; //To determine the position of insert in internal node
        e->ptr = newNode;
        e->dupli = 1; //No new key in block
        key prev;
        prev.value = -1;
        prev.type = NOT_IN_USE;
        e->prev = prev;
        return newNode;
    }

    //Key is new
    e->key = (&newNode->keys[0])->value;
    e->ptr = newNode;
    e->dupli = 0; //Have new key which is also the first element

    // This checks for the case where the creation of the new node causes the original node to only contain duplicate keys
    // In this case, other than inserting a new key (for the new node) into the parent node, we might also have to
    // amend the value and type of the previous key
    internalNodesCtr = 0;
    if (origNode != search(ROOT, (&origNode->keys[0])->value, &internalNodesCtr)) {
        key prev;
        prev.value = (&origNode->keys[0])->value;
        prev.type = REPEATED;
        e->prev = prev;
    } else {
        key prev;
        prev.value = (&origNode->keys[0])->value;
        prev.type = FIRST_OCCURENCE;
        e->prev = prev;
    }
    return newNode;
}

// Leaf node insert without splitting
Node* _noSplitLeaf(Node* origNode, double_t key_value, RecordPtr* record, Element* e) {
    // Find index to insert key at
    int i = 0;
    while (i < MAX_KEYS && (&origNode->keys[i])->type != NOT_IN_USE && (&origNode->keys[i])->value <= key_value) {
        i++;
    }

    // Shift keys and pointers above index to the right
    for (int j = MAX_KEYS-1; j > i; j--) {
        origNode->keys[j] = origNode->keys[j-1];
        origNode->children[j] = origNode->children[j-1];
    }

    // Insert new key and pointer into empty slot
    (&origNode->keys[i])->value = key_value;
    (&origNode->keys[i])->type = FIRST_OCCURENCE;
    origNode->children[i] = record;
    origNode->numKeys++;

    //Key is not new
    e->key = -1;
    e->ptr=NULL;
    e->dupli=1;
    key prev;
    prev.value = -1;
    prev.type = NOT_IN_USE;
    e->prev = prev;

    return origNode;
}

// Internal splitting
Node* _splitInternal(Node* origNode, Element* e, int insertionIdx) {
    //Create internal node
    Node* newNode = createNode(0);

    // We use temporary lists to store the keys and children pointers prior to shifting
    key tempKeys[MAX_KEYS + 1];
    struct Node* tempPtrs[MAX_KEYS + 2];

    for (int i = 0; i < MAX_KEYS; i++) {
        tempKeys[i] = origNode->keys[i];
        tempPtrs[i] = origNode->children[i];
    }
    tempPtrs[MAX_KEYS] = origNode->children[MAX_KEYS];

    int i = insertionIdx;

    // Shift keys and pointers above index to the right
    for (int j = MAX_KEYS; j > i; j--) {
        tempKeys[j] = tempKeys[j - 1];
    }

    for (int j = MAX_KEYS + 1; j > i; j--) {
        tempPtrs[j] = tempPtrs[j-1];
    }

    // Insert new key and pointer into empty slot
    if(e->dupli==0){
        (&tempKeys[i])->value = e->key;
        (&tempKeys[i])->type = FIRST_OCCURENCE;
    }else{
        (&tempKeys[i])->value = e->key;
        (&tempKeys[i])->type = REPEATED;
    }
    tempPtrs[i + 1] = e->ptr;

    //The Key at first pos of new node is removed, ptr is retained
    int remNodePos = ceil((MAX_KEYS)/2.0);
    newNode->children[0] = tempPtrs[remNodePos+1];
    (&origNode->keys[remNodePos])->type = NOT_IN_USE;
    origNode->children[remNodePos+1]=NULL;

    //Insert all other keys and their pointers
    for(int i=remNodePos+1, j=0; i<MAX_KEYS+1; i++, j++){
        newNode->keys[j] = tempKeys[i];
        newNode->children[j+1] = tempPtrs[i+1];
        (&origNode->keys[i])->type = NOT_IN_USE;
        origNode->children[i+1]=NULL;
    }

    for(int i=0; i<remNodePos; i++){
        origNode->keys[i]=tempKeys[i];
        origNode->children[i]=tempPtrs[i];
    }
    origNode->children[remNodePos]=tempPtrs[remNodePos];

    //Adjust the number of keys in children
    origNode->numKeys=(int) ceil(MAX_KEYS/2.0);
    newNode->numKeys=floor((MAX_KEYS)/2.0);

    //Manipulate values of e
    int k=remNodePos;
    while(k < MAX_KEYS + 1 && (&tempKeys[k])->type != NOT_IN_USE){
        if((&tempKeys[k])->type != REPEATED){
            e->key = (&tempKeys[k])->value;
            e->ptr = newNode;
            key prev;
            prev.value = -1;
            prev.type = NOT_IN_USE;
            e->prev = prev;
            return newNode;
        }
        k++;
    }
    
    //No new keys in subtree, use largest non null in tempKeys as proxy
    k=remNodePos-1;
    while(k>=0){
        if((&tempKeys[k])->type != REPEATED){
            e->key = (&tempKeys[k])->value; //Proxy heuristic
            e->ptr = newNode;
            e->dupli=1; //NULL entry
            key prev;
            prev.value = -1;
            prev.type = NOT_IN_USE;
            e->prev = prev;
            return newNode;
        }
        k--;
    }

    //If all nodes in subtree are NULL, use the current value of e->key as proxy 
    e->ptr = newNode;
    e->dupli=1; //NULL entry
    key prev;
    prev.value = -1;
    prev.type = NOT_IN_USE;
    e->prev = prev;
    return newNode;
}

// Non Internal splitting
Node* _noSplitInternal(Node* origNode, Element* e, int insertionIdx) {
    // Find index to insert key at
    Node* cur = e->ptr;

    int i = insertionIdx;

    // Shift keys and pointers above index to the right
    for (int j = MAX_KEYS-1; j > i; j--) {
        origNode->keys[j] = origNode->keys[j - 1];
        origNode->children[j + 1] = origNode->children[j];
    }

    // Insert new key and pointer into empty slot
    if(e->dupli==0){
        (&origNode->keys[i])->value = e->key;
        (&origNode->keys[i])->type = FIRST_OCCURENCE;
    }else{
        (&origNode->keys[i])->value = e->key;
        (&origNode->keys[i])->type = REPEATED;
    }
    
    origNode->children[i + 1] = e->ptr;
    origNode->numKeys++;

    if ((&e->prev)->type != NOT_IN_USE && i != 0) {
        (&origNode->keys[i-1])->value = (&e->prev)->value;
        (&origNode->keys[i-1])->type = (&e->prev)->type;
    }

    // Fix nodes
    key curKey = origNode->keys[0];
    for (int j = 1; j < origNode->numKeys; j++) {
        if ((&origNode->keys[j])->value == (&curKey)->value){
            (&origNode->keys[j])->type = REPEATED;
        } else {
            curKey = origNode->keys[j];
            (&origNode->keys[j])->type = FIRST_OCCURENCE;
        }
    }


    //No splits made
    e->key=-1;
    e->ptr=NULL;
    e->dupli=0;
    key prev;
    prev.value = -1;
    prev.type = NOT_IN_USE;
    e->prev = prev;

    return origNode;
}

//Recursively insert and return the pointer to the modified node
Node* insert(Node* cursor, double_t key, RecordPtr* record, Element* e){
    if(cursor->isLeaf==1){
        if(cursor->numKeys==MAX_KEYS){
            return _splitLeaf(cursor, key, record, e);
        }else{
            return _noSplitLeaf(cursor, key, record, e);
        }
    }

    // Find index to recurse 'down'
    int i = 0;
    while (i < MAX_KEYS && (&cursor->keys[i])->value <= key && (&cursor->keys[i])->type != NOT_IN_USE) {
        i++;
    }

    //Recurse down
    Node* child = insert(cursor->children[i], key, record, e);

    //Modify the current node by splits, if needed
    if(e->ptr!=NULL){
        if(cursor->numKeys==MAX_KEYS){
            return _splitInternal(cursor, e, i);
        }
        else{
            return _noSplitInternal(cursor, e, i);
        }
    } else {
        if ((&cursor->keys[i - 1])->type == REPEATED && (&cursor->keys[i - 1])->value != key) {
            (&cursor->keys[i - 1])->value = key;
            (&cursor->keys[i - 1])->type = FIRST_OCCURENCE;
        }
    }

    return cursor;
}

// For more debugging and visualisation.
// Differs from printTree in that this finds the extreme left node and follows
// only the leaf node pointers to print all the leaves
void printLeaves(Node* root) {
    Node* cur = root;
    printf("Leaves:\t ");
    while (cur != NULL) {
        if (!cur->isLeaf) {
            cur = cur->children[0];
        } else {
            for (int i = 0; i < MAX_KEYS; i++) {
                if ((&cur->keys[i])->type != NOT_IN_USE) {
                    printf("%.3lf ", (&cur->keys[i])->value);
                } else {
                    printf("X ");
                }
            }
            printf("\t");
            cur = cur->children[MAX_KEYS];
        }
    }
}

void printTree(Node* cursor, int level) {     // For basic debugging/visualization
    if (cursor == NULL) {
        return;
    }

    printf("Level %d: ", level);

    if (cursor->isLeaf==1) {
        for (int i = 0; i < MAX_KEYS; i++) {    // Print all keys (if empty/null, will print -1/-2)
            if ((&cursor->keys[i])->type == NOT_IN_USE) {
                printf("X ");
            } else if ((&cursor->keys[i])->type == REPEATED) {
                printf("- ");
            } else {
                printf("%.3lf ", (&cursor->keys[i])->value);
            }
        }
        printf(" Keys: %d\n",cursor->numKeys);
    }
    else {
        for (int i = 0; i < MAX_KEYS; i++) { 
            if ((&cursor->keys[i])->type == NOT_IN_USE) {
                printf("X ");
            } else if ((&cursor->keys[i])->type == REPEATED) {
                printf("- ");
            } else {
                printf("%.3lf ", (&cursor->keys[i])->value);
            }
        }
        printf(" Keys: %d\n",cursor->numKeys);
        for (int i = 0; i < cursor->numKeys + 1; i++) {
            printTree(cursor->children[i], level + 1);
        }
    }
}

//counts number of nodes (both leaf and non-leaf nodes) in the tree and returns it
int countNumNodes(Node *root) {

    Node* tempNode = root;
    int count = 1;

    if (root->isLeaf) {
        return count;
    }

    for (int i = 0; i < tempNode->numKeys + 1; i++) {
        count += countNumNodes((Node *)tempNode->children[i]);
    }

    return count;
}

//counts number of levels in the tree and returns it
int countNumLevels(Node *root) {

    Node* tempNode = root;
    int levels = 0;
    
    while(!tempNode->isLeaf) {
        tempNode = (Node *)tempNode->children[0];
        levels++;
    }

    levels++;
    return levels;
}

//check how many leafNodes are present in the parent node, parent node can be of higher levels as well
int countLeafNodes(Node *node) {

    int noOfLeafNodes = 0;

    if (((Node*)node->children[0])->isLeaf) {
        noOfLeafNodes = node->numKeys + 1;
        return noOfLeafNodes;
    } else {
        //recusively checks for number of leafNodes in each respective pointers
        for (int i = 0; i < node->numKeys + 1; i ++ ) {
            noOfLeafNodes += countLeafNodes((Node *)node->children[i]);
        }
    }
    return noOfLeafNodes;
}

//when deleting, the subsequent parent keys will not change!

void _delete(Node *node, int index);
int _remedy(Node *node, int childIndex, Node **rootNode);
void _merge(Node *leftNode, Node *rightNode);
void _transfer(Node *leftNode, Node *rightNode, int unitsTransferred);
void _updateKey(Node *node, int index);
int countLeafNodes(Node *node);

//main delete function, take note that it only works with queries < Key 
int deleteQuery(double_t FG_PCT_home, Node *node, Node **rootNode) {

    //isParentCheckRequired is to let algorithm know if need to return to parent node to check for correctness of B+ tree
    int isParentCheckRequired;
    //escapeKey is to let algorithm know that delete query is done and can now exit the resursive function
    int escapeKey;

    if (node->isLeaf) {
        if ((&node->keys[0])->value > FG_PCT_home && ((&node->keys[0])->type != NOT_IN_USE)) {
            return 1;
        }
        while ((&node->keys[0])->value <= FG_PCT_home && node->numKeys != 0 && ((&node->keys[0])->type != NOT_IN_USE)) {
            _delete(node, 0);
        }
        return 0;
    } else {
        escapeKey = deleteQuery(FG_PCT_home, (Node *)node->children[0], rootNode);
    }
    while(!escapeKey) {
        isParentCheckRequired = _remedy(node, 0, rootNode);
    
        if (isParentCheckRequired) {
            return 0;
        } else {
            escapeKey = deleteQuery(FG_PCT_home, (Node *)node->children[0], rootNode);
        }
    }

    return 1;
}

// After deletion, fix leaf connections
void fixLeaves(Node* root) {
    Node* cur = root;
    Node* start = NULL;
    Node* end = NULL;
    while (cur != NULL) {
        if (!cur->isLeaf) {
            cur = cur->children[0];
        } else {
            if (start == NULL) {
                start = cur;
            }
            Node* nextNode = (Node *)cur->children[MAX_KEYS];
            if (start == NULL && nextNode != NULL) {
                int numKeys = 0;
                for (int i = 0; i < MAX_KEYS; i++) {
                    if ((&nextNode->keys[i])->type != NOT_IN_USE) {
                        numKeys++;
                    }
                }
                if (numKeys == 0) {
                    start = cur;
                }
            }
            if (start != NULL && end == NULL && nextNode != NULL) {
                int numKeys = 0;
                for (int i = 0; i < MAX_KEYS; i++) {
                    if ((&nextNode->keys[i])->type != NOT_IN_USE) {
                        numKeys++;
                    }
                }
                if (numKeys != 0) {
                    end = nextNode;
                }
            }
            cur = cur->children[MAX_KEYS];
        }
    }
    if (start != NULL) {
        start->children[MAX_KEYS] = end;
    }
    return;
}

//only deletes the key in the node and it's pointer at the same index, if leafnode, will also delete the record from main memory
void _delete(Node *node, int index) {

    int sizeOfArray = sizeof(node->keys) / sizeof(node->keys[0]);

    if (node->isLeaf) {
        //insert delete record block here
        delete_from_block(((RecordPtr *)node->children[index])->blockPtr, ((RecordPtr *)node->children[index])->offset, 0);
    }

    //shift keys, and pointers 1 index to the left
    while (index < node->numKeys) {
        if (index + 1 == sizeOfArray) {
            //ignore the value, change the type
            (&node->keys[index])->type = NOT_IN_USE;
        } else {
            (&node->keys[index])->value = (&node->keys[index + 1])->value;
            (&node->keys[index])->type = (&node->keys[index + 1])->type;
        }

        node->children[index] = node->children[index + 1];
        index++;
    }

    //only if node has no more keys, then should not decrease further to -1 and just exit function instead
    if (node->numKeys != 0) {
        node->numKeys--;
    }

    if (node->isLeaf) {
        if (index == sizeOfArray) {
            node->children[index - 1] = NULL;
        }
    } else {
        node->children[index] = NULL;
    }

    return;
}

//function to edit the tree due to the structure being wrong, implements _transfer/_merge of nodes and returns 0/1 to let main algorithm know if
//checking of parent node is required or not
int _remedy(Node *node, int childIndex, Node **rootNode) {

    int sizeOfArray = sizeof(node->keys) / sizeof(node->keys[0]);
    int noOfKeys = ((Node *)node->children[childIndex])->numKeys;
    int noOfPtrs = 0;
    int minKeys;

    //determines minimum keys based on leafNode or nonLeafNode
    if (((Node *)node->children[childIndex])->isLeaf) {
        minKeys = floor((sizeOfArray + 1) / 2);
    } else {
        minKeys = floor(sizeOfArray / 2);
    }

    //checks if node assumption of keys is violated and if so, _remedy is applied
    if (minKeys > noOfKeys) {
        if (childIndex == 0 && node->children[childIndex + 1] != NULL) {
            if (floor((noOfKeys + ((Node *)node->children[childIndex + 1])->numKeys) / 2 ) < minKeys) {
                _merge((Node *)node->children[childIndex], (Node *)node->children[childIndex + 1]);
                free(((Node *)node->children[childIndex + 1]));
                _delete(node, childIndex + 1);
                _updateKey(node, childIndex);
            } else  {
                if (((Node *)node->children[childIndex])->isLeaf) {
                    _transfer((Node *)node->children[childIndex], (Node *)node->children[childIndex + 1], minKeys - noOfKeys);
                } else {
                    for (int i = 0; i < ((Node *)node->children[childIndex])->numKeys + 1; i++) {
                        if (((Node *)node->children[childIndex])->children[i] == NULL) {
                            break;
                        }
                        noOfPtrs++;
                    }
                    _transfer((Node *)node->children[childIndex], (Node *)node->children[childIndex + 1], (minKeys + 1) - noOfPtrs);
                    _updateKey(node, 0);
                }
            }
        } else if (childIndex != 0) {
            if (floor((noOfKeys + ((Node *)node->children[childIndex - 1])->numKeys) / 2) > minKeys) {
                if (((Node *)node->children[childIndex])->isLeaf) {
                    _transfer((Node *)node->children[childIndex - 1], (Node *)node->children[childIndex], minKeys - noOfKeys);
                } else {
                    for (int i = 0; i < ((Node *)node->children[childIndex])->numKeys + 1; i++) {
                        if (((Node *)node->children[childIndex])->children[i] == NULL) {
                            break;
                        }
                        noOfPtrs++;
                    }
                    _transfer((Node *)node->children[childIndex - 1], (Node *)node->children[childIndex], (minKeys + 1) - noOfPtrs);
                    _updateKey(node, childIndex - 1);
                }
            } else if (childIndex != noOfKeys - 1 && floor((noOfKeys + ((Node *)node->children[childIndex + 1])->numKeys) / 2) > minKeys){
                if (((Node *)node->children[childIndex])->isLeaf) {
                    _transfer((Node *)node->children[childIndex], (Node *)node->children[childIndex + 1], minKeys - noOfKeys);
                } else {
                    for (int i = 0; i < ((Node *)node->children[childIndex])->numKeys + 1; i++) {
                        if (((Node *)node->children[childIndex])->children[i] == NULL) {
                            break;
                        }
                        noOfPtrs++;
                    }
                    _transfer((Node *)node->children[childIndex], (Node *)node->children[childIndex + 1], (minKeys + 1) - noOfPtrs);
                    _updateKey(node, childIndex);
                }
            } else {
                _merge((Node*)node->children[childIndex - 1], (Node *)node->children[childIndex]);
                free(((Node *)node->children[childIndex - 1]));
                _delete(node, childIndex - 1);
            }
        }

        //on this special case that the node is root node and no more keys, then it will _transfer the root node position to it's only child pointer left
        //which should be on the first index
        if (node == *rootNode) {
            if (node->numKeys == 0) {
                *rootNode = node->children[0];
                setGlobal(node->children[0]);
                //free(node);
            }
            return 0;
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}

//_merge the nodes together, node to be deleted is the node with smaller number of keys
void _merge(Node *leftNode, Node *rightNode) {

    int largerKeyValue;
    int noOfPtrs = 0;

    if (leftNode->numKeys > rightNode->numKeys) {
        largerKeyValue =  leftNode->numKeys;
    } else {
        largerKeyValue = rightNode->numKeys;
    }

    if (leftNode->isLeaf) {
        _transfer(leftNode, rightNode, largerKeyValue);
    } else if (leftNode->numKeys > rightNode->numKeys) {
        for (int i = 0; i < leftNode->numKeys + 1; i++) {
            if (leftNode->children[i] == NULL) {
                break;
            }
            noOfPtrs++;
        }
        _transfer(leftNode, rightNode, noOfPtrs);
    } else {
        for (int i = 0; i < rightNode->numKeys + 1; i++) {
            if (rightNode->children[i] == NULL) {
                break;
            }
            noOfPtrs++;
        }
        _transfer(leftNode, rightNode, noOfPtrs);
    }
}

//_transfer pointers/keys from the node with higher number of keys to node with smaller number of keys
void _transfer(Node *leftNode, Node *rightNode, int unitsTransferred) {    

    if (unitsTransferred == 0) {
        return;
    }

    if (leftNode->numKeys < rightNode->numKeys) {
        if (leftNode->isLeaf) {
            while(unitsTransferred > 0) {
                (&leftNode->keys[leftNode->numKeys])->value = (&rightNode->keys[0])->value;
                (&leftNode->keys[leftNode->numKeys])->type = (&rightNode->keys[0])->type;
                leftNode->children[leftNode->numKeys] = rightNode->children[0];
                _delete(rightNode, 0);
                leftNode->numKeys++;
                unitsTransferred--;
            }
        } else {
            leftNode->children[leftNode->numKeys + 1] = rightNode->children[0];
            _updateKey(leftNode, leftNode->numKeys);
            leftNode->numKeys++;
            unitsTransferred--;
            while(unitsTransferred > 0) {
                (&leftNode->keys[leftNode->numKeys])->value = (&rightNode->keys[0])->value;
                (&leftNode->keys[leftNode->numKeys])->type = (&rightNode->keys[0])->type;
                leftNode->children[leftNode->numKeys + 1] = rightNode->children[1];
                _delete(rightNode, 0);
                leftNode->numKeys++;
                unitsTransferred--;
            }
            _delete(rightNode, 0);
        }
    } else {
        if (leftNode->isLeaf) {
            while(unitsTransferred > 0) {
                for (int i = rightNode->numKeys; i > 0; i--) {
                    (&rightNode->keys[i])->value = (&rightNode->keys[i - 1])->value;
                    (&rightNode->keys[i])->type = (&rightNode->keys[i - 1])->type;
                    rightNode->children[i] = rightNode->children[i - 1];
                }
                (&rightNode->keys[0])->value = (&leftNode->keys[leftNode->numKeys - 1])->value;
                (&rightNode->keys[0])->type = (&leftNode->keys[leftNode->numKeys - 1])->type;
                rightNode->children[0] = leftNode->children[leftNode->numKeys - 1];
                _delete(leftNode, leftNode->numKeys - 1);
                rightNode->numKeys++;
                unitsTransferred--;
            }
        } else {
            rightNode->children[rightNode->numKeys + 1] = rightNode->children[rightNode->numKeys];
            for (int i = rightNode->numKeys; i > 0; i--) {
                (&rightNode->keys[i])->value = (&rightNode->keys[i - 1])->value;
                (&rightNode->keys[i])->type = (&rightNode->keys[i - 1])->type;
                rightNode->children[i] = rightNode->children[i - 1];
            }
            _updateKey(rightNode, 0);
            rightNode->children[0] = leftNode->children[leftNode->numKeys];
            leftNode->children[leftNode->numKeys] = NULL;
            rightNode->numKeys++;
            unitsTransferred--;
            while(unitsTransferred > 0) {
                rightNode->children[rightNode->numKeys + 1] = rightNode->children[rightNode->numKeys];
                for (int i = rightNode->numKeys; i > 0; i--) {
                    (&rightNode->keys[i])->value = (&rightNode->keys[i - 1])->value;
                    (&rightNode->keys[i])->type = (&rightNode->keys[i - 1])->type;
                    rightNode->children[i] = rightNode->children[i - 1];
                }
                (&rightNode->keys[0])->value = (&leftNode->keys[leftNode->numKeys - 1])->value;
                (&rightNode->keys[0])->type = (&leftNode->keys[leftNode->numKeys - 1])->type;
                rightNode->children[0] = leftNode->children[leftNode->numKeys - 1];
                _delete(leftNode, leftNode->numKeys - 1);
                rightNode->numKeys++;
                unitsTransferred--;
            }
            //change the type, ignore the value
            (&leftNode->keys[leftNode->numKeys - 1])->type = NOT_IN_USE;
            leftNode->numKeys--;
        }
    }
}

//checks the smallest key value of the right pointer (index + 1) as well as possibilities of duplicate key
//in this case, will update key regardless of it's previous value
//take note this function assumes that the tree is structurally correct except for the key of that index, and it should only update the nonLeafNodes
void _updateKey(Node *node, int index) {

    //tempNode is one of the leafNodes found in the right pointer of parent node
    Node* tempNode;
    //tempNode2 is the left leafNode of the leafNode that has the smallest value in the right pointer of the parent node
    //used to check for duplication of keys in the leafNodes in the right pointer of parent node
    Node* tempNode2;
    int noOfLeafNodes;

    //check in case key updated is on the far right of nonLeadNode (where right pointer that key does not exist)
    if (node->children[index + 1] == NULL) {
        (&node->keys[index])->type = NOT_IN_USE;
        return;
    } 

    if (((Node *)node->children[index])->isLeaf) {
        tempNode = (Node *)node->children[index];
        if ((&tempNode->keys[tempNode->numKeys - 1])->value != (&((Node *)node->children[index + 1])->keys[0])->value) {
            (&node->keys[index])->value = (&((Node *)node->children[index + 1])->keys[0])->value;
            (&node->keys[index])->type = (&((Node *)node->children[index + 1])->keys[0])->type;
        } else {
            tempNode2 = (Node *)node->children[index + 1];
            for (int i = 1; i < tempNode2->numKeys; i++) {
                if ((&tempNode2->keys[0])->value != (&tempNode2->keys[i])->value) {
                    (&node->keys[index])->value = (&tempNode2->keys[i])->value;
                    (&node->keys[index])->type = (&tempNode2->keys[i])->type;
                    return;
                }
            }
            (&node->keys[index])->value = (&tempNode2->keys[0])->value;
            (&node->keys[index])->type = REPEATED;
        }
    } else {
        tempNode = (Node *)node->children[index];
        while (!tempNode->isLeaf) {
            tempNode = (Node *)tempNode->children[tempNode->numKeys];
        }
        
        tempNode2 = (Node *)node->children[index + 1];
        while (!tempNode2->isLeaf) {
            tempNode2 = (Node *)tempNode2->children[0];
        }

        noOfLeafNodes = countLeafNodes((Node *)node->children[index + 1]);

        while (noOfLeafNodes != 0) {
            for (int i = 0; i < tempNode2->numKeys; i++) {
                if ((&tempNode->keys[tempNode->numKeys - 1])->value != (&tempNode2->keys[i])->value) {
                    (&node->keys[index])->value = (&tempNode2->keys[i])->value;
                    (&node->keys[index])->type = (&tempNode2->keys[i])->type;
                    return;
                }
            }
            //goes to next leafNode on the right
            tempNode2 = (Node *)tempNode2->children[MAX_KEYS];
            noOfLeafNodes--;
        }
        (&node->keys[index])->value = (&tempNode->keys[tempNode->numKeys - 1])->value;
        (&node->keys[index])->type = REPEATED;
        return;
    }

}