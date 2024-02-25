#ifndef LINEAR_SCAN_H
#define LINEAR_SCAN_H

#include "block.h"
#include "types.h"
#include "record.h"

// Search for a key using linear scan
double_t linearScanPoint(byte_t *disk[], int blocksInUse, double_t target);

// Search for a range of keys using linear scan
double_t linearScanRange(byte_t *disk[], int blocksInUse, double_t target_lower, double_t target_upper);

// Delete all records with key value less than or equal to target key
void linearScanDelete(byte_t *disk[], int blocksInUse, double_t target);

#endif