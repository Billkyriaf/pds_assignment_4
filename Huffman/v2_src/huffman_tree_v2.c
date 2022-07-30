#include <stdlib.h>
#include <string.h>
#include "huffman_tree_v2.h"


/**
 * Counts the character frequency of every ascii char of the file being compressed
 *
 * @param file  The file to count the frequencies
 * @param asciiHuffman  The huffman struct
 */
void charFrequency(FILE *file, ASCIIHuffman *asciiHuffman) {

    uint8_t c;  // The character read from the file

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
 *
 */
void appendBitToSymbol(Symbol *sym, uint8_t bit){
    // The symbol_length DIV 8 gives the byte index that the new bit will be appended
    uint8_t byte_index = sym->symbol_length / 8;

    // The symbol_length MOD 8 gives the position in the byte the new bit will be inserted
    uint8_t insert_position = sym->symbol_length % 8;

    // Since C does not have a rotate operator the bits are shifted left so that the bit ands up in the position to be inserted
    bit = bit << (7 - insert_position);

    sym->symbol_length++; // increase the length of the symbol by 1

    // To append the ne bit we simply need to add the shifted bit to the byte that the symbol was inserted.
    sym->symbol[byte_index] += bit;
}


/**
 * Initializes the leaf nodes of the tree.
 *
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
            nodes[nodes_index].left = -1;         // The index of the left child is -1 because there is no left child
            nodes[nodes_index].right = -1;        // The index of the right child is -1 because there is no right child

            for (int j = 0; j < 32; ++j) {
                nodes[nodes_index].leaf_symbol.symbol[j] = 0;   // The huffman symbol initially is 0
            }
            nodes[nodes_index].leaf_symbol.symbol_length = 0;   // The symbol length is initially 0

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
void printTree(HuffmanNode *nodes, uint16_t nodes_index) {
    printf("Huffman Tree: \n");

    for (int i = nodes_index + 1; i >= 0; --i) {

        if (nodes[i].isLeaf) {
            printf("    LeafNode: %d, freq: %lu, symbol: 0x", i, nodes[i].freq);

            for (int j = 0; j < SYMBOL_BYTES; ++j) {
                printf("%x", nodes[i].leaf_symbol.symbol[j]);
            }

            printf(", character: %c\n", nodes[i].ascii_index);

        } else {
            printf("Node: %d, freq: %lu, left child %d, right child %d\n", i, nodes[i].freq, nodes[i].left,
                   nodes[i].right);
        }
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
     * The maximum nodes a huffman tree with 256 different symbols can have is 511. The nodes array holds all the nodes
     * of the tree. The symbols are updated as the tree builds.
     */
    HuffmanNode nodes[511];
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

/**
 * Creates a huffman tree from an array with the huffman symbols. The depth of every symbol is determined from the
 * number of bits is has. The more bits the deeper on the tree
 *
 * @param asciiHuffman  The symbols array
 * @param tree          The array that holds all the nodes
 * @return              The index of the top node
 */
uint8_t huffmanFromArray(ASCIIHuffman *asciiHuffman, HuffmanNode *tree){
    uint8_t tree_index = 0;
    uint16_t len_mask = 0xF;  // mask to get the symbol length

    // Create all the leaf nodes
    for (int i = 0; i < 256; ++i) {
        if (asciiHuffman->symbols[i] != 0){
            tree[tree_index].leaf_symbol = asciiHuffman->symbols[i];     // The huffman symbol initially is 0
            tree[tree_index].freq = asciiHuffman->symbols[i] & len_mask; // Character frequency is replaced by the length of the symbol
            tree[tree_index].ascii_index = i;   // The character
            tree[tree_index].isLeaf = true;     // These nodes are the leaf nodes of the tree
            tree[tree_index].isInTree = false;  // The node is in the array, but it is not in the tree yet
            tree[tree_index].left = -1;         // The index of the left child is -1 because there is no left child
            tree[tree_index].right = -1;        // The index of the right child is -1 because there is no right child

            tree_index++;  // increment the index
        }
    }

    // Find the two nodes with the biggest symbols
}