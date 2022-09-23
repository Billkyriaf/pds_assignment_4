#ifndef FILE_UTILS_PTH_H
#define FILE_UTILS_PTH_H

#include <cstdio>

#include "../../include/structs.h"

/**
 * Opens a file in binary format for reading
 *
 * @param filename The file name
 * @param mode     The mode to open the file
 * @return  The FILE pointer created
 */
FILE *openBinaryFile(const char *filename, const char *mode);


/**
 * Divides the frequency calculation work between threads and then creates the threads to actually calculate the
 * frequencies
 *
 * @param filename The input file name (to be compressed)
 * @param huffman  The huffman struct
 */
void calculateFrequency(const char *filename, ASCIIHuffman *huffman);


/**
 * Takes the file to be compressed reads the bits and creates a new compressed file.
 *
 * @param file       The original file
 * @param filename   The filename of the compressed file
 * @param huffman    The huffman struct that contains the information for the compression
 * @param block_size  The size of the data that every write operation writes to the file
 */
void compressFile(const char *filename, ASCIIHuffman *huffman, uint16_t block_size);


/**
 * Decompresses a file
 *
 * @param filename  The name of the file to be decompressed
 */
void decompressFile(const char *filename);

#endif //FILE_UTILS_PTH_H
