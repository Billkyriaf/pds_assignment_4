#ifndef CHAR_FREQUENCY_CILK_H
#define CHAR_FREQUENCY_CILK_H

#include "../structs.h"

/**
 * Divides the frequency calculation work between threads and then creates the threads to actually calculate the
 * frequencies
 *
 * @param filename The input file name (to be compressed)
 * @param huffman  The huffman struct
 */
void calculateFrequency(const std::string& filename, ASCIIHuffman *huffman) ;

#endif
