#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <cstdio>
#include "huffman_tree_pthread.h"


/**
 * Opens a file in binary format for reading
 *
 * @param filename The file name
 * @return  The FILE pointer created
 */
FILE *openBinaryFile(char *filename);


/**
 * Takes the file to be compressed reads the bits and creates a new compressed file.
 *
 * @param file       The original file
 * @param filename   The filename of the compressed file
 * @param huffman    The huffman struct that contains the information for the compression
 * @param blockSize  The size of the data that every write operation writes to the file
 */
void compressFile(FILE *file, char *filename, ASCIIHuffman *huffman, uint16_t blockSize);


/**
 * Decompresses a file
 *
 * @param filename  The name of the file to be decompressed
 */
void decompressFile(char *filename);

#endif //FILE_UTILS_H
