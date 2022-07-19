#include "huffman_tree.h"
#include <stdbool.h>

/**
 * @brief  The huffman_node struct represent a node in the huffman tree. The node can be a leaf node or a regular node
 *
 */
typedef struct huffman_node{
    uint16_t leaf_symbol;  /// The huffman symbol
    uint8_t ascii_index;  /// The ascii char
    unsigned int freq;  /// The frequency of the character

    bool isLeaf;  /// True if the node is a leaf node
    bool isInTree;  /// True if the node is part of the tree

    uint16_t left;  /// The index of the left child of the node (-1 if there are no children)
    uint16_t right;  /// The index of the right child of the node (-1 if there are no children)

} HuffmanNode;


/**
 * Counts the character frequency of every ascii char of the file being compressed
 * @param file  The file to count the frequencies
 * @param asciiHuffman  The huffman struct
 */
void charFrequency(FILE *file, ASCIIHuffman *asciiHuffman) {

    uint8_t c;  // The character read from the file

    // TODO handle non aligned data

    fseek(file, 0, SEEK_END);  // Jump to the end of the file
    long int file_len = ftell(file);  // Get the current byte offset in the file

    rewind(file);  // Jump back to the beginning of the file

    // Read from the file byte by byte
    for (int i = 0; i < file_len; ++i) {
        fread(&c, sizeof(c), 1, file);

        // For every char read update the frequency
        asciiHuffman->charFreq[c]++;
    }
}


/**
 * Prepends 1 bit to the symbol 16bit representation number. The symbol format is the following
 *
 *    MSB 15... .... ...4 3..0  LSB
 *         0000 0000 0000 0000
 *
 *    bits 0:3   store the information of how many bits is the huffman symbol
 *    bits 4:15  store the huffman symbol
 *
 *    eg:
 *        symbol     = 0000 0000 0111 0101  That means that the symbol is 5bits long and the symbol is 00111
 *
 *    Prepend bit 1 to symbol:
 *
 *        new_symbol = 0000 0010 0111 0110  The 1 was added to the bit 9 (prepended to the symbol which already
 *                                                                        had 5 bits, bit 4 to bit 8)
 *
 * @param symbol  The symbol to update
 * @param bit     The bit to add (0 or 1)
 * @return        The updated symbol
 */
uint16_t prependBitToSymbol(uint16_t symbol, uint16_t bit){
    uint16_t mask = 0xF;  // mask is set to 0000 0000 0000 1111

    /*
     * This is the number of shift bits in order to add a new bit to the symbol. symbol & 0000_0000_0000_1111 gives the
     * last 4 bits of the symbol that represent the number of bits the symbol has. If we add 4 to this we get the number
     * of left shifts that we need to perform on bit variable in order for the nwe bit to come to the position we want it
     */
    uint16_t shiftBits = (symbol & mask) + 4;

    symbol += 1;  // +1 increases the number of bits part (last 4 bits) of the symbol

    /*
     * Shifting the bit by shiftBits produces a mask with the new bit to the position.
     *
     * eg.
     *     Say we want to add 1 to this symbol 0000 0000 0111 0101. the shiftBits is 0000 0000 0000 0101 + 4 which is 9
     *     in decimal. The symbol part of the 16 bit value is 00111 and the new symbol will be 100111.
     *
     *     The bit = 0000 0000 0000 0001 so by shifting it by shiftBits = 9 we get 0000 0010 0000 0000
     *
     *     The symbol is incremented by 1 (symbol + 1) = 0000 0000 0111 (0110) -> 6 is the length of the new symbol
     *     So now we only need to add the bit with the symbol to get the new symbol:
     *
     *                                          bit  0000 0010 0000 0000
     *                                   +   symbol  0000 0000 0111 0110
     *                                   ────────────────────────────────
     *                                   new_symbol  0000 0010 0111 0110
     */
    bit = bit << shiftBits;

    return bit + symbol;
}


/**
 * Appends 1 bit to the symbol 16bit representation number. The symbol format is the following
 *
 *    MSB 15... .... ...4 3..0  LSB
 *         0000 0000 0000 0000
 *
 *    bits 0:3   store the information of how many bits is the huffman symbol
 *    bits 4:15  store the huffman symbol
 *
 *    eg:
 *        symbol     = 0000 0000 0111 0101  That means that the symbol is 5bits long and the symbol is 00111
 *
 *        Append bit 0 to symbol:
 *
 *        new_symbol = 0000 0000 1110 0110  The 0 was added to the bit 4 (appended to the symbol)
 *
 *
 * @param symbol  The symbol to update
 * @param bit     The bit to add (0 or 1)
 * @return        The updated symbol
 */
uint16_t appendBitToSymbol(uint16_t symbol, uint16_t bit){
    uint16_t mask = 0xFFF0;  // mask is set to 1111 1111 1111 0000

    symbol += 1;  // +1 increases the number of bits part (last 4 bits) of the symbol

    /*
     * To append the bit to the symbol we get the symbol with the help of the mask. Then the new symbol is shifted left
     * by one to make room for the new bit, the bit is sifted 4 places to align, and then it is added.
     *
     *      eg.  Initial symbol 0000 0000 0011 0010  and bit = 0000 0000 0000 0001
     *
     *         symbol & mask         = 0000 0000 0011 0000
     *
     *
     *     (symbol & mask) << 1      = 0000 0000 0110 0000
     *    +      bit << 4            = 0000 0000 0001 0000
     *    ──────────────────────────────────────────────────
     *           new_symbol          = 0000 0000 0111 0000
     */
    uint16_t new_symbol = symbol & mask;
    new_symbol = new_symbol << 1;
    new_symbol += (bit << 4);

    // Get the 4 LSB from the old symbol and add them to the new symbol

    mask = 0xF;  // mask is set to 0000 0000 0000 1111

    // Symbol & mask gives the 4 LSB of the old symbol.
    return new_symbol + (symbol & mask);
}


/**
 * Initializes the leaf nodes of the tree.
 * @param asciiHuffman  The ASCIIHuffman struct
 * @param nodes         The nodes array
 * @return              The index of the next node to be inserted in the array
 */
uint16_t initializeTree(ASCIIHuffman *asciiHuffman, HuffmanNode *nodes){
    uint16_t nodes_index = 0;

    // init the leaf nodes
    for (int i = 0; i < 256; ++i) {

        // If the character is present (frequency not 0)...
        if (asciiHuffman->charFreq[i] != 0){
            nodes[nodes_index].freq = asciiHuffman->charFreq[i];  // Character frequency
            nodes[nodes_index].ascii_index = i;   // The character
            nodes[nodes_index].isLeaf = true;     // These nodes are the leaf nodes of the tree
            nodes[nodes_index].isInTree = false;  // The node is in the array, but it is not in the tree yet
            nodes[nodes_index].leaf_symbol = 0;   // The huffman symbol initially is 0
            nodes[nodes_index].left = -1;         // The index of the left child is -1 because there is no left child
            nodes[nodes_index].right = -1;        // The index of the right child is -1 because there is no right child

            // Increment the index of the tree array
            nodes_index++;
        }
    }

    // Return the index of the next node
    return nodes_index;
}


/**
 * Prints the huffman tree
 *
 * @param nodes        The nodes array of the tree
 * @param nodes_index  The index of the last node in the tree
 */
void printTree(HuffmanNode *nodes, uint16_t nodes_index){
    printf("Huffman Tree: \n");

    for (int i = nodes_index + 1; i >= 0; --i) {

        if (nodes[i].isLeaf)
            printf("    LeafNode: %d, freq: %u, symbol: 0x%x, character: %c\n",
                   i, nodes[i].freq, nodes[i].leaf_symbol, nodes[i].ascii_index);
        else
            printf("Node: %d, freq: %u, left child %d, right child %d\n", i, nodes[i].freq, nodes[i].left, nodes[i].right);
    }
}


/**
 * Recursive function that updates the huffman symbols of the leaf nodes. The nodes are updated from the top to the
 * bottom so every time the function is called the new bit is added to the LSB of the symbol.
 *
 * @param asciiHuffman  The final Huffman array
 * @param rootNode      The root node of the huffman tree
 * @param symbol        The huffman symbol up to that depth
 * @param nodes         The array of the tree nodes
 */
void updateSymbols(ASCIIHuffman *asciiHuffman, HuffmanNode *rootNode, uint16_t symbol, HuffmanNode *nodes){

    // Termination condition. If the node is a leaf node ...
    if (rootNode->isLeaf){
        // ... update the symbol in the tree
        rootNode->leaf_symbol = symbol;

        // And update the symbol in the final huffman struct
        asciiHuffman->symbols[rootNode->ascii_index] = symbol;
        return;
    }

    // If the node is not a leaf node the function will be recursively called for the left and right child

    // The left and right symbols are created from the current symbol...
    uint16_t left_symbol = symbol;
    uint16_t right_symbol = symbol;

    // ... by appending the bit 0 to the left symbol
    left_symbol = appendBitToSymbol(left_symbol, 0);

    // ... and the bit 1 to the right symbol
    right_symbol = appendBitToSymbol(right_symbol, 1);

    // The function is called recursively for the left ...
    updateSymbols(asciiHuffman, &nodes[rootNode->left], left_symbol, nodes);

    // ...and for the right child
    updateSymbols(asciiHuffman, &nodes[rootNode->right], right_symbol, nodes);
}


/**
 * The main part of the huffman algorithm. This function calculates all the symbols for the characters that exist (have
 * a frequency of more than 1)
 * @param asciiHuffman  The huffman struct with the character frequencies
 */
void calculateSymbols(ASCIIHuffman *asciiHuffman){
    /*
     * The maximum nodes a huffman tree with 256 different symbols can have is 1023. The nodes array holds all the nodes
     * of the tree. The symbols are updated as the tree builds.
     */
    HuffmanNode nodes[1023];
    uint16_t nodes_index;  // The index of the last node inserted

    // Init the array with the leaf nodes
    nodes_index = initializeTree(asciiHuffman, nodes);

    // The nodes in the array that are not in the tree
    uint16_t remaining_nodes = nodes_index;

    // find the nodes with the two smallest frequencies in the nodes array.

    while (remaining_nodes > 1){

        uint16_t s1_index = -1;  // The smallest frequency index in the frequency array
        uint16_t s2_index = -1;  // The second-smallest frequency index in the frequency array

        // The smallest frequencies at each point of the algorithm
        unsigned int s1_freq = UINT32_MAX;
        unsigned int s2_freq = UINT32_MAX;

        // find the first and second-smallest indexes
        for (int i = 0; i < nodes_index; ++i) {

            // If the node is not in the tree already...
            if (!nodes[i].isInTree) {
                // Read the current frequency from memory
                unsigned int freq = nodes[i].freq;

                if (freq < s1_freq && freq != 0) {
                    s2_index = s1_index;
                    s2_freq = s1_freq;

                    s1_index = i;
                    s1_freq = freq;

                } else if (freq < s2_freq && freq != 0) {
                    s2_index = i;
                    s2_freq = freq;
                }
            }
        }

        // Create a new node
        nodes[nodes_index].freq = s1_freq + s2_freq;  // The sum of the children frequencies
        nodes[nodes_index].isLeaf = false;            // This is not a leaf node
        nodes[nodes_index].isInTree = false;          // The node is in the array, but it is not in the tree yet
        nodes[nodes_index].left = s1_index;           // The index of the left child
        nodes[nodes_index].right = s2_index;          // The index of the right child

        // Update the child nodes
        nodes[s1_index].isInTree = true;  // The first child is in the tree
        nodes[s2_index].isInTree = true;  // The second child is in the tree

        remaining_nodes--;  // Two old nodes are in the tree and one new is not
        nodes_index++;  // Increment the index of the tree array

    }

    // After the tree is complete all the symbols are updated
    updateSymbols(asciiHuffman, &nodes[--nodes_index], 0, nodes);

    // FIXME remove print
    printTree(nodes, --nodes_index);
}