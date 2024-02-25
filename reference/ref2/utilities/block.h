/**
 * Defines block I/O functions
*/

#ifndef BLOCK_H
#define BLOCK_H

#include "types.h"

extern const size_t BLOCK_METADATA_SIZE;

/** @brief Write data to the first available space in a block
 * 
 *  @param blockPtr Pointer to the start of the block
 *  @param data Data that should be written to the block
 *  @param data_size The size of the data in bytes 
 *  @param verbose Whether debugging output should be printing (Set to 0 to suppress output)
 * 
 *  @return Pointer to the start of the block
 */
byte_t* write_to_block(byte_t *blockPtr, byte_t data[], size_t data_size, int verbose);

/** @brief Read data from block starting at a specified position
 * 
 *  @param blockPtr Pointer to the start of the block
 *  @param start The starting index of the data to be read
 *  @param data Pointer to the location in which to store the read data
 *  @param verbose Whether debugging output should be printing (Set to 0 to suppress output)
 * 
 *  @return Pointer to the start of the block
 */
byte_t* read_from_block(byte_t *blockPtr, int start, byte_t *data, int verbose);

/** @brief Delete data from block starting at a specified position
 * 
 *  @param blockPtr Pointer to the start of the block
 *  @param start The starting index of the data to be deleted
 *  @param verbose Whether debugging output should be printing (Set to 0 to suppress output)
 * 
 *  @return Pointer to the start of the block
 */
byte_t* delete_from_block(byte_t *blockPtr, int start, int verbose);

#endif