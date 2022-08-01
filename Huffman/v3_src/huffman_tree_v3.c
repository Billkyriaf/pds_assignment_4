#include <stdlib.h>
#include "huffman_tree_v3.h"

#define SCAN_SIZE 10 * 102400  // 100MB

/**
 * Counts the character frequency of every ascii char of the file being compressed. The ascii characters are 8 bits long
 * This produces potentially long symbols that are hard to manage. For this reason every char is split in two 4 bit parts
 * This produces 16 different chars and a maximum length of huffman symbols of 15.
 *
 * Since the number of different characters is low the huffman tree can be calculated for all the characters even if they
 * are not present. This allows to measure the frequency of the characters on a small part of the file and assume the
 * file is relatively consistent. The larger the area of the file accounted the more accurate the results but the longer
 * it takes to measure
 *
 * @param file     The file to count the frequencies
 * @param huffman  The huffman struct
 */
void charFrequency(FILE *file, HalfASCIIHuffman *huffman) {

    uint8_t c;  // The character read from the file

    fseek(file, 0, SEEK_END);  // Jump to the end of the file
    unsigned long int file_len = ftell(file);  // Get the current byte offset in the file

    rewind(file);  // Jump back to the beginning of the file

    uint8_t MSBs;  // The "upper" half of the character
    uint8_t LSBs;  // The "lower" half of the character

    uint8_t msb_mask = 0xF0;
    uint8_t lsb_mask = 0x0F;

    unsigned long int scan_size = file_len;

//    // Determine the scan_size of the file
//    if (file_len > 2 * SCAN_SIZE)
//        scan_size = SCAN_SIZE;
//    else
//        scan_size = file_len;

    // Read from the file byte by byte
    for (unsigned long int i = 0; i < scan_size; ++i) {
        fread(&c, sizeof(c), 1, file);

        /*
         * To get the "upper" half of the char AND it with 1111 0000 and shift the result right 4 positions. This creates
         * a 4 bit number ("half ascii")
         */
        MSBs = (c & msb_mask) >> 4;

        /*
         * Similar for the "lower" half of the number. Only now the number is already only 4 bits so no shifting is
         * required.
         */
        LSBs = c & lsb_mask;

        // For every half char read update the frequency
        huffman->charFreq[MSBs]++;
        huffman->charFreq[LSBs]++;
    }
}


/**
 * Shifts the symbol by 1 to the left and adds the new symbol in the lsb position of the symbol
 *
 * @param sym   Pointer to the symbol to update
 * @param bit   The bit to append
 */
void appendBitToSymbol(Symbol *sym, uint8_t bit){
    // Left shift the symbol to make space for the new bit
    sym->symbol = (sym->symbol << 1) + bit;

    // Update the symbol length
    sym->symbol_length += 1;
}


/**
 * Initializes the leaf nodes of the tree.
 *
 * @param huffman  The HalfASCIIHuffman struct
 * @param nodes         The nodes array
 * @return              The index of the next node to be inserted in the array
 */
uint16_t initializeTree(HalfASCIIHuffman *huffman, HuffmanNode *nodes){
    uint16_t nodes_index = 0;

    // init the leaf nodes
    for (int i = 0; i < 16; ++i) {
        nodes[nodes_index].freq = huffman->charFreq[i];  // Character frequency
        nodes[nodes_index].ascii_index = i;   // The character
        nodes[nodes_index].isLeaf = true;     // These nodes are the leaf nodes of the tree
        nodes[nodes_index].isInTree = false;  // The node is in the array, but it is not in the tree yet
        nodes[nodes_index].left = -1;         // The index of the left child is -1 because there is no left child
        nodes[nodes_index].right = -1;        // The index of the right child is -1 because there is no right child

        nodes[nodes_index].leaf_symbol.symbol = 0;  // The huffman symbol initially is 0
        nodes[nodes_index].leaf_symbol.symbol_length = 0;   // The symbol length is initially 0

        // Increment the index of the tree array
        nodes_index++;
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

    for (int i = nodes_index; i >= 0; --i) {

        if (nodes[i].isLeaf) {
            printf("    LeafNode: %d, freq: %lu, symbol: 0x", i, nodes[i].freq);
            printf("%x", nodes[i].leaf_symbol.symbol);
            printf(", character: %u\n", nodes[i].ascii_index);

        } else {
            printf("Node: %d, freq: %lu, left child %d, right child %d\n", i, nodes[i].freq, nodes[i].left,
                   nodes[i].right);
        }
    }
}


/**
 * Recursive function that updates the huffman symbols of the leaf nodes. The nodes are updated from the top to the
 * bottom so every time the function is called the new bit is added to the LSB of the sym.
 *
 * @param asciiHuffman  The final Huffman array
 * @param rootNode      The root node of the huffman tree
 * @param sym           The huffman symbol up to that depth
 * @param nodes         The array of the tree nodes
 */
void updateSymbols(HalfASCIIHuffman *asciiHuffman, HuffmanNode *rootNode, Symbol sym, HuffmanNode *nodes){

    // Termination condition. If the node is a leaf node ...
    if (rootNode->isLeaf){
        // And update the sym in the final huffman struct
        asciiHuffman->symbols[rootNode->ascii_index] = sym;

        return;
    }

    // If the node is not a leaf node the function will be recursively called for the left and right child

    // The left and right symbols are created from the current sym...
    Symbol left_symbol = sym;
    Symbol right_symbol = sym;

    // ... by appending the bit 0 to the left sym
    appendBitToSymbol(&left_symbol, 0);

    // ... and the bit 1 to the right sym
    appendBitToSymbol(&right_symbol, 1);

    if (nodes[rootNode->left].isInTree) {
        // The function is called recursively for the left ...
        updateSymbols(asciiHuffman, &nodes[rootNode->left], left_symbol, nodes);
    }

    if (nodes[rootNode->right].isInTree) {
        // ...and for the right child
        updateSymbols(asciiHuffman, &nodes[rootNode->right], right_symbol, nodes);
    }
}


/**
 * The main part of the huffman algorithm. This function calculates all the symbols for the characters that exist (have
 * a frequency of more than 1)
 * @param asciiHuffman  The huffman struct with the character frequencies
 */
void calculateSymbols(HalfASCIIHuffman *asciiHuffman){
    /*
     * The maximum nodes a huffman tree with 16 different symbols can have is 31. The nodes array holds all the nodes
     * of the tree. The symbols are updated as the tree builds.
     */
    HuffmanNode nodes[31];
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
        uint64_t s1_freq = UINT64_MAX;
        uint64_t s2_freq = UINT64_MAX;

        // find the first and second-smallest indexes
        for (int i = 0; i < nodes_index; ++i) {

            // If the node is not in the tree already...
            if (!nodes[i].isInTree) {
                // Read the current frequency from memory
                unsigned int freq = nodes[i].freq;

                if (freq < s1_freq) {
                    s2_index = s1_index;
                    s2_freq = s1_freq;

                    s1_index = i;
                    s1_freq = freq;

                } else if (freq < s2_freq) {
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

    // Initial symbol all zero
    Symbol sym;

    sym.symbol_length = 0;
    sym.symbol = 0;

    // After the tree is complete all the symbols are updated
    updateSymbols(asciiHuffman, &nodes[nodes_index - 1], sym, nodes);

    // FIXME remove print
    printTree(nodes, nodes_index - 1);
}

/**
 * Creates a huffman tree from an array with the huffman symbols. The depth of every symbol is determined from the
 * number of bits is has. The more bits the deeper on the tree
 *
 * @param asciiHuffman  The symbols array
 * @param tree          The array that holds all the nodes
 * @return              The index of the top node
 */
uint8_t huffmanFromArray(HalfASCIIHuffman *asciiHuffman, HuffmanNode *tree){
    uint8_t tree_index = 0;
    uint16_t len_mask = 0xF;  // mask to get the symbol length

    // Create all the leaf nodes
//    for (int i = 0; i < 256; ++i) {
//        if (asciiHuffman->symbols[i] != 0){
//            tree[tree_index].leaf_symbol = asciiHuffman->symbols[i];     // The huffman symbol initially is 0
//            tree[tree_index].freq = asciiHuffman->symbols[i] & len_mask; // Character frequency is replaced by the length of the symbol
//            tree[tree_index].ascii_index = i;   // The character
//            tree[tree_index].isLeaf = true;     // These nodes are the leaf nodes of the tree
//            tree[tree_index].isInTree = false;  // The node is in the array, but it is not in the tree yet
//            tree[tree_index].left = -1;         // The index of the left child is -1 because there is no left child
//            tree[tree_index].right = -1;        // The index of the right child is -1 because there is no right child
//
//            tree_index++;  // increment the index
//        }
//    }

    // Find the two nodes with the biggest symbols
}