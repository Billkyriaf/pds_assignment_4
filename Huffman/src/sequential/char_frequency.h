#ifndef CHAR_FREQUENCY_H
#define CHAR_FREQUENCY_H

#include "../structs.h"

/**
 * Counts the character frequency of every ascii char of the file being compressed
 *
 * @param filename  The file name to count the frequencies from
 * @param huffman   The huffman struct
 */
void charFrequency(const char *filename, ASCIIHuffman *huffman);

#endif
