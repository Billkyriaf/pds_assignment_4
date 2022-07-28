#ifndef HUFFMAN_TREE
#define HUFFMAN_TREE

#include <stdio.h>
#include <inttypes.h>

/**
 * In ascii huffman the bits of the file are translated as ascii characters (1 byte each). So there are 256 possible
 * symbols. The file doesn't have to be ascii characters because the data are managed in binary
 */
typedef struct ascii_huffman {

    /**
     * The symbols array stores the encoded huffman symbol for every possible character. The uint16_t numbers have the
     * following format:
     *
     *    MSB 15... .... ...4 3..0  LSB
     *         0000 0000 0000 0000
     *
     *    bits 0:3   store the information of how many bits is the huffman symbol
     *    bits 4:15  store the huffman symbol
     *
     *    eg:
     *
     *        symbols[0] = 0000 0000 0111 0101  That means that the symbol is 5bits long and the symbol is 00111
     */
    uint16_t symbols[256];

    /**
     * Counts the frequency of every symbol
     */
    unsigned long int charFreq[256];

} ASCIIHuffman;

/**
 * Counts the character frequency of every ascii char of the file being compressed
 * @param file  The file to count the frequencies
 * @param asciiHuffman  The huffman struct
 */
void charFrequency(FILE *file, ASCIIHuffman *asciiHuffman);

/**
 * The main part of the huffman algorithm. This function calculates all the symbols for the characters that exist (have
 * a frequency of more than 1)
 * @param asciiHuffman  The huffman struct with the character frequencies
 */
void calculateSymbols(ASCIIHuffman *asciiHuffman);

#endif //HUFFMAN_TREE
