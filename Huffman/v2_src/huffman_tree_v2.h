#ifndef HUFFMAN_TREE
#define HUFFMAN_TREE

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>


/**
 * The max number of bytes a symbol can have. If n is the number of different characters then the max possible length of
 * a huffman symbol is n - 1 bits. In ascii there are 256 different characters so in the worst case 255 bits are needed.
 * The closest number that creates full bytes is 256 bits which is 32 bytes
 */
#define SYMBOL_BYTES 32


/**
 * This is a single symbol for an ascii character. Every character is 8bits long. That means there are 256 different
 * characters. This leads to huffman symbols with with max length of 255bits. This is a special limit case but we need
 * to account for it.
 *
 */
typedef struct symbol {
    /**
     * The symbol is an array of 32 unsigned 8bit integers. All together it is a 256bit sequence of bits. The bytes are
     * stored in little endian (LSB in position 0 of the array) format, and the bits of every byte are stored in little
     * endian format as well. With this setup the symbol is stored in reverse in memory.
     *
     *    eg
     *
     *    symbol length: 10 bits
     *
     *           index: 0          1         ... 30         31
     *    symbol array: 1101 0101  0100 0000 ... 0000 0000  0000 0000
     *
     *    The symbol is: 1010101011  -> The first bit is the second bit of the 2nd byte and the last bit is the first
     *                                  bit of the first byte.
     */
    uint8_t symbol[SYMBOL_BYTES];

    /// The symbol length
    uint8_t symbol_length;

} Symbol;


/**
 * In ascii huffman the bits of the file are translated as ascii characters (1 byte each). So there are 256 possible
 * symbols. The file doesn't have to be ascii characters because the data are managed in binary
 */
typedef struct ascii_huffman {

    /**
     * The symbols array stores the encoded huffman symbol for every possible character.
     */
    Symbol symbols[256];

    /**
     * The frequency of every symbol
     */
    unsigned long int charFreq[256];

    /**
     * The number of bytes of the longest symbol in the symbols array. This is used for limiting the space needed to
     * store the huffman array in the compressed file
     */
    uint8_t longest_symbol;
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
 * Counts the character frequency of every ascii char of the file being compressed
 *
 * @param file  The file to count the frequencies
 * @param asciiHuffman  The huffman struct
 */
void charFrequency(FILE *file, ASCIIHuffman *asciiHuffman);

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
uint8_t huffmanFromArray(ASCIIHuffman *asciiHuffman, HuffmanNode *tree);

#endif //HUFFMAN_TREE
