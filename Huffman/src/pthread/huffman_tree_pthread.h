#ifndef HUFFMAN_TREE
#define HUFFMAN_TREE

#include <cstdio>
#include <cinttypes>

#include "../../libs/uint256_t.h"

/**
 * This is a single symbol for an ascii character. Every character is 4bits long. That means there are 16 different
 * characters. This leads to huffman symbols with with max length of 15 bits.
 */
typedef struct symbol {
    /**
     * The huffman symbol of every character. The symbols are created from the bottom to the top of the tree. As a
     * result first bit of the symbol is added last so it can be found at the LSB of the 16 bit number.
     *
     *      eg
     *          uint16_t symbol: 0000 0010 1010 1011
     *          symbol length: 10
     *
     *          To reach the leaf node from the root node we take this path 1, 1, 0, 1, 0, 1, 0, 1, 0, 1
     */
    uint256_t symbol;

    /// The symbol length measured in bits
    uint8_t symbol_length;

} Symbol;


/**
 * In half ascii huffman the bits of the file are translated as half ascii characters (4 bits each). So there are 16
 * possible "characters". The file doesn't have to be ascii characters because the data are managed in binary
 */
typedef struct half_ascii_huffman {

    /**
     * The symbols array stores the encoded huffman symbol for every possible character.
     */
    Symbol symbols[256];

    /**
     * The frequency of every symbol
     */
    uint64_t charFreq[256];

} ASCIIHuffman;


/**
 * The huffman_node struct represent a node in the huffman tree. The node can be a leaf node or a regular node
 *
 */
typedef struct huffman_node{
    Symbol leaf_symbol;  /// The huffman symbol
    uint8_t ascii_index;  /// The ascii char
    unsigned long int freq;  /// The frequency of the character

    bool isLeaf;  /// True if the node is a leaf node
    bool isInTree;  /// True if the node is part of the tree

    uint16_t left;  /// The index of the left child of the node (-1 if there are no children)
    uint16_t right;  /// The index of the right child of the node (-1 if there are no children)

} HuffmanNode;


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
void calculateSymbols(ASCIIHuffman *asciiHuffman);


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
