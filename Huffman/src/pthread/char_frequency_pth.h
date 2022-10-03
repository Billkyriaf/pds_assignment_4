#ifndef CHAR_FREQUENCY_PTH_H
#define CHAR_FREQUENCY_PTH_H

#include "../structs.h"


/**
 * Divides the frequency calculation work between threads and then creates the threads to actually calculate the
 * frequencies
 *
 * @param filename The input file name (to be compressed)
 * @param huffman  The huffman struct
 */
void calculateFrequency(const char *filename, ASCIIHuffman *huffman);


#endif
