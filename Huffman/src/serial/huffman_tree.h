#ifndef HUFFMAN_TREE
#define HUFFMAN_TREE

#include <cstdio>

#include "../../include/structs.h"

/**
 * Counts the character frequency of every ascii char of the file being compressed
 *
 * @param filename  The file name to count the frequencies from
 * @param huffman   The huffman struct
 */
void charFrequency(const char *filename, ASCIIHuffman *huffman);


/**
 * Prints the huffman tree
 *
 * @param nodes        The nodes array of the tree
 * @param nodes_index  The index of the last node in the tree
 */
void printTree(HuffmanNode *nodes, uint16_t nodes_index);


/**
 * The main part of the huffman algorithm. This function calculates all the symbols for the characters that exist (have
 * a frequency of more than 1)
 *
 * @param asciiHuffman  The huffman struct with the character frequencies
 */
void createHuffmanTree(ASCIIHuffman *asciiHuffman);


/**
 * Creates a huffman tree from an array with the huffman symbols. The depth of every symbol is determined from the
 * number of bits is has. The more bits the deeper on the tree
 *
 * @param asciiHuffman  The symbols array
 * @param tree          The array that holds all the nodes
 * @return              The index of the top node
 */
uint16_t huffmanFromArray(ASCIIHuffman *huffman, HuffmanNode *tree);

#endif //HUFFMAN_TREE
