#ifndef COMPRESS_H
#define COMPRESS_H

#include "../structs.h"

/**
 * Takes the file to be compressed reads the bits and creates a new compressed file.
 *
 * @param file       The original file
 * @param filename   The filename of the compressed file
 * @param huffman    The huffman struct that contains the information for the compression
 * @param block_size  The size of the data that every write operation writes to the file
 */
void compressFile(const char *filename, ASCIIHuffman *huffman, uint16_t block_size);

#endif
