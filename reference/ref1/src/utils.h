#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <chrono>
#include <vector>

#include "disk.h"

using namespace std;
using namespace chrono;

tuple<int, int> exp3LinearScan(Disk disk,int linearScanBlocksAccessed);
tuple<int,int> exp4LinearScan(Disk disk, float lower, float upper,int linearScanBlocksAccessed);
tuple<int,int> exp5LinearScan(Disk disk, float keysToDeleteBelow,int linearScanBlocksAccessed);

#endif