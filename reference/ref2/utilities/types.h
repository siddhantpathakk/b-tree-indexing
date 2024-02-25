#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>
#include <stdlib.h>
#include <math.h>

#define MB 1048576
#define PERCENTAGE_DECIMAL_PLACES 1000
#define MAX_KEYS 16 // n

static const size_t DISK_CAPACITY = 100 * MB;
static const size_t BLOCK_SIZE = 400;
static const int num_of_blocks = DISK_CAPACITY / BLOCK_SIZE;

typedef uint8_t byte_t;

typedef struct record {
    byte_t data[14];
} record;

enum chunk_state {
    FREE,
    IN_USE,
    END_OF_BLOCK
};

typedef enum {
    NOT_IN_USE, // Indicates a space in a Node that is not in use. Previously -1
    REPEATED, // Indicates a space in a Node that should be a NULL (or '-' in the textbook). Previously -2
    FIRST_OCCURENCE // Indicates a space in a Node that is a normal key
} key_type;

// Wrapper to store key values and whether they should be considered as duplicated
typedef struct key {
    double_t value;
    key_type type;
} key;

typedef struct _{
    byte_t *blockPtr;
    int16_t offset;
} RecordPtr;

typedef struct __{
    double_t key;
    void* ptr;
    int dupli; // 1 if -2(duplicated) flag has to be inserted
    key prev; // Used to determine if the previous key needs to be changed, and if so, to what key and type
} Element;

typedef struct ___ {
    int16_t isLeaf;
    int16_t numKeys;
    key keys[MAX_KEYS];
    void* children[MAX_KEYS + 1];    // Can point to either RecordPtr (if Node is leaf) or child node (if Node is internal)
                                    // If leaf, children[MAX_KEYS] points to next leaf node
} Node;

#endif