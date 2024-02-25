#include "types.h"
#include <stdio.h>

const size_t BLOCK_METADATA_SIZE = 2;

byte_t* _format_block(byte_t *blockPtr, size_t data_size) {
    // Use an extra byte to indicate if the space is being used
    size_t chunk_size = data_size + 1;

    if (chunk_size > BLOCK_SIZE) {
        printf("Error. Data size (%dB) specified cannot fit into block with Size (%dB)", data_size, BLOCK_SIZE);
        return NULL;
    };

    // Store block metadata in the first BLOCK_METADATA_SIZE bytes
    uint64_t bitmask = 0b11111111; // Note: Allow at most 8 bytes of metadata
    uint8_t bitshift = 0;
    for (int i = BLOCK_METADATA_SIZE; i > 0; i--) {
        blockPtr[i - 1] = (chunk_size & bitmask) >> bitshift;
        bitmask <<= 8;
        bitshift += 8;
    }

    for (int i = 2; i < BLOCK_SIZE; i++) {
        if (i + chunk_size > BLOCK_SIZE) {
            blockPtr[i] = END_OF_BLOCK;
        } else {
            blockPtr[i] = FREE;
        }
    }

    return blockPtr;
}

byte_t* _unformat_block(byte_t *blockPtr) {
    for (int i = BLOCK_METADATA_SIZE; i > 0; i--) {
        blockPtr[i - 1] = 0;
    }

    return blockPtr;
}

size_t _get_block_chunk_size(byte_t *blockPtr) {
    size_t chunk_size = 0;
    uint8_t bitshift = 0;
    for (int i = BLOCK_METADATA_SIZE; i > 0; i--) {
        chunk_size = (blockPtr[i - 1] << bitshift) | chunk_size;
        bitshift += 8;
    }

    return chunk_size;
}

int _is_block_empty(byte_t *blockPtr) {
    size_t chunk_size = _get_block_chunk_size(blockPtr);

    for (int i = BLOCK_METADATA_SIZE; i < BLOCK_SIZE; i += chunk_size) {
        if (blockPtr[i] == IN_USE) {
            return 0;
        }
    };

    return 1;
}


byte_t* write_to_block(byte_t *blockPtr, byte_t data[], size_t data_size, int verbose) {
    size_t chunk_size = _get_block_chunk_size(blockPtr);
    
    if (chunk_size == 0) {
        if (blockPtr == (_format_block(blockPtr, data_size))) {
            chunk_size = _get_block_chunk_size(blockPtr);
        }
    } 

    if (chunk_size != data_size + 1) {
        if (verbose) printf("Cannot write data of size %dB to formatted block of chunk size %dB\n", data_size, chunk_size);
        return NULL;
    }

    for (int i = BLOCK_METADATA_SIZE; i < BLOCK_SIZE; i += chunk_size) {
        if (blockPtr[i] == FREE) {
            for (int j = 0; j < data_size; j++) {
                blockPtr[i + 1 + j] = data[j];
            }
            blockPtr[i] = IN_USE;
            return blockPtr;
        }
    };

    return NULL;
};

byte_t* delete_from_block(byte_t *blockPtr, int start, int verbose) {
    size_t chunk_size = _get_block_chunk_size(blockPtr);

    if (chunk_size == 0) {
        if (verbose) printf("Cannot delete data from unformatted block\n");
        return NULL;
    }

    if ((start - BLOCK_METADATA_SIZE) % chunk_size != 0) {
        if (verbose) printf("Cannot delete data at position %d when block chunk size is %dB\n", start, chunk_size);
        return NULL;
    }

    if (start > BLOCK_SIZE) {
        if (verbose) printf("Cannot delete data at position %d when block size is %dB\n", start, BLOCK_SIZE);
        return NULL;
    }

    for (int i = 1; i < chunk_size; i++) {
        blockPtr[start + i] = 0;
    }

    blockPtr[start] = FREE;

    // Check if the block is empty. If it is, unformat it.
    if (_is_block_empty(blockPtr)) {
        blockPtr = _unformat_block(blockPtr);
    }

    return blockPtr;
}

byte_t* read_from_block(byte_t *blockPtr, int start, byte_t *data, int verbose) {
    size_t chunk_size = _get_block_chunk_size(blockPtr);

    if (chunk_size == 0) {
        if (verbose) printf("Cannot read data from unformatted block\n");
        return NULL;
    }

    if ((start - BLOCK_METADATA_SIZE) % chunk_size != 0) {
        if (verbose) printf("Cannot read data at position %d when block chunk size is %dB\n", start, chunk_size);
        return NULL;
    }

    if (start > BLOCK_SIZE) {
        if (verbose) printf("Cannot read data at position %d when block size is %dB\n", start, BLOCK_SIZE);
        return NULL;
    }

    if (blockPtr[start] != IN_USE) {
        if (verbose) printf("No data found at position %d\n", start);
        return NULL;
    } 

    for (int i = 1; i < chunk_size; i++) {
        data[i - 1] = blockPtr[start + i];
    };

    return data;
};