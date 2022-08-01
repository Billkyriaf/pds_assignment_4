#ifndef HUFFMAN_TREE
#define HUFFMAN_TREE

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>


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
    uint16_t symbol;

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
    Symbol symbols[16];

    /**
     * The frequency of every symbol
     */
    uint64_t charFreq[16];

} HalfASCIIHuffman;


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
 * Counts the character frequency of every ascii char of the file being compressed
 *
 * @param file  The file to count the frequencies
 * @param huffman  The huffman struct
 */
void charFrequency(FILE *file, HalfASCIIHuffman *huffman);

/**
 * The main part of the huffman algorithm. This function calculates all the symbols for the characters that exist (have
 * a frequency of more than 1)
 *
 * @param asciiHuffman  The huffman struct with the character frequencies
 */
void calculateSymbols(HalfASCIIHuffman *asciiHuffman);


/**
 * Creates a huffman tree from an array with the huffman symbols. The depth of every symbol is determined from the
 * number of bits is has. The more bits the deeper on the tree
 *
 * @param asciiHuffman  The symbols array
 * @param tree          The array that holds all the nodes
 * @return              The index of the top node
 */
uint8_t huffmanFromArray(HalfASCIIHuffman *asciiHuffman, HuffmanNode *tree);

#endif //HUFFMAN_TREE
