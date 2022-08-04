#ifndef HUFFMAN_TREE_PTH_H
#define HUFFMAN_TREE_PTH_H

#include <cstdio>

#include "../structs.h"

/**
 * Counts the character frequency of every ascii char of the file being compressed. The file is simultaneously handled
 * by multiple threads each having it's own handler. Every thread reads and counts a different part of the file, from
 * start_byte to end byte
 *
 * @param file           The file to count the frequencies
 * @param frequency_arr  The frequency array of the characters
 * @param start_byte     The byte (inclusive, measuring from 0) from where to start counting in the file
 * @param end_byte       The byte (exclusive, measuring from 0) to where to stop counting in the file
 */
void charFrequency(FILE *file, uint64_t *frequency_arr, uint64_t start_byte, uint64_t end_byte);


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

#endif //HUFFMAN_TREE_PTH_H
