#include <iostream>
#include <cmath>
#include "huffman_tree.h"

#define SCAN_SIZE 1 * 1024 * 1024 * 1024  // 1GB
//#define DEBUG_MODE


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
void charFrequency(FILE *file, ASCIIHuffman *huffman) {

    uint8_t c;  // The character read from the file

    fseek(file, 0, SEEK_END);  // Jump to the end of the file
    unsigned long int file_len = ftell(file);  // Get the current byte offset in the file

    rewind(file);  // Jump back to the beginning of the file

    uint64_t scan_size = file_len;

/*    // Determine the scan_size of the file
    if (file_len > SCAN_SIZE)
        scan_size = SCAN_SIZE;
    else
        scan_size = file_len;*/

    // Read from the file byte by byte
    for (unsigned long int i = 0; i < scan_size; ++i) {
        fread(&c, sizeof(c), 1, file);

        // For every half char read update the frequency
        huffman->charFreq[c]++;
    }
}


/**
 * Shifts the bit by length to the left and adds the new bit in the msb position of the symbol
 *
 * @param sym   Pointer to the symbol to update
 * @param bit   The bit to append
 */
void addBitToSymbol(Symbol *sym, uint8_t bit) {

    // Left shift the symbol to make space for the new bit
    sym->symbol = (sym->symbol << 1) + bit;

    // Update the symbol length
    sym->symbol_length += 1;
}


/**
 * Initializes the leaf nodes of the tree.
 *
 * @param huffman  The ASCIIHuffman struct
 * @param nodes         The nodes array
 * @return              The index of the next node to be inserted in the array
 */
uint16_t initializeTree(ASCIIHuffman *huffman, HuffmanNode *nodes) {
    uint16_t nodes_index = 0;

    // init the leaf nodes
    for (int i = 0; i < 256; ++i) {
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
            std::cout << "    LeafNode: " << i << ", freq: " << nodes[i].freq << ", symbol: 0x" << std::hex
                      << nodes[i].leaf_symbol.symbol << ", character: " << std::dec <<nodes[i].ascii_index << std::endl;

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
void updateSymbols(ASCIIHuffman *asciiHuffman, HuffmanNode *rootNode, Symbol sym, HuffmanNode *nodes) {

    // Termination condition. If the node is a leaf node ...
    if (rootNode->isLeaf) {
        // And update the sym in the final huffman struct
        asciiHuffman->symbols[rootNode->ascii_index] = sym;
        rootNode->leaf_symbol = sym;
        return;
    }

    // If the node is not a leaf node the function will be recursively called for the left and right child

    // The left and right symbols are created from the current sym...
    Symbol left_symbol = sym;
    Symbol right_symbol = sym;

    // ... by appending the bit 0 to the left sym
    addBitToSymbol(&left_symbol, 0);

    // ... and the bit 1 to the right sym
    addBitToSymbol(&right_symbol, 1);

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
void calculateSymbols(ASCIIHuffman *asciiHuffman) {
    /*
     * The maximum nodes a huffman tree with 16 different symbols can have is 31. The nodes array holds all the nodes
     * of the tree. The symbols are updated as the tree builds.
     */
    HuffmanNode nodes[511];
    uint16_t nodes_index;  // The index of the last node inserted

    // Init the array with the leaf nodes
    nodes_index = initializeTree(asciiHuffman, nodes);

    // The nodes in the array that are not in the tree
    uint16_t remaining_nodes = nodes_index;

    // find the nodes with the two smallest frequencies in the nodes array.

    while (remaining_nodes > 1) {

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

#ifdef DEBUG_MODE
    printTree(nodes, nodes_index - 1);
#endif
}

/**
 * Creates a huffman tree from an array with the huffman symbols. The depth of every symbol is determined from the
 * number of bits is has. The more bits the deeper on the tree
 *
 * @param asciiHuffman  The symbols array
 * @param tree          The array that holds all the nodes
 * @return              The index of the top node
 */
uint16_t huffmanFromArray(ASCIIHuffman *huffman, HuffmanNode *tree) {
    uint16_t tree_index = 0;

    // Create all the leaf nodes
    for (int i = 0; i < 256; ++i) {
        tree[tree_index].leaf_symbol = huffman->symbols[i];     // The huffman symbol initially is 0
        tree[tree_index].ascii_index = i;   // The character
        tree[tree_index].isLeaf = true;     // These nodes are the leaf nodes of the tree
        tree[tree_index].isInTree = false;  // The node is in the array, but it is not in the tree yet
        tree[tree_index].left = -1;         // The index of the left child is -1 because there is no left child
        tree[tree_index].right = -1;        // The index of the right child is -1 because there is no right child

        tree_index++;  // increment the index
    }

    uint16_t ind1;  // index of the first child node
    uint16_t ind2;  // index of the second child node
    uint8_t max_l;  // the length of the biggest symbol

    do {
        // reset the values
        ind1 = 0xffff;  // No node with index 0xffff can exist
        ind2 = 0xffff;
        max_l = 0;

        // find the symbol with the maximum length
        for (int i = 0; i < tree_index; ++i) {
            if (!tree[i].isInTree){
                if (tree[i].leaf_symbol.symbol_length > max_l) {
                    max_l = tree[i].leaf_symbol.symbol_length;
                    ind1 = i;
                }
            }
        }

        // Find the neighbour symbol. The neighbor symbol has the same length and only differs on the last bit
        for (int i = 0; i < tree_index; ++i) {
            if (!tree[i].isInTree && i != ind1 && tree[i].leaf_symbol.symbol_length == max_l) {
                if ((tree[ind1].leaf_symbol.symbol >> 1) == (tree[i].leaf_symbol.symbol >> 1)) {
                    ind2 = i;
                    break;  // once the symbol is found break the loop
                }
            }
        }

        /*
         * The parent node has a hypothetical symbol that is 1 bit less than it's children.
         * The bit missing is thew last bit of the children's symbol
         */
        tree[tree_index].leaf_symbol.symbol = tree[ind1].leaf_symbol.symbol >> 1;
        tree[tree_index].leaf_symbol.symbol_length = tree[ind1].leaf_symbol.symbol_length - 1;

        tree[tree_index].isLeaf = false;     // This is not a leaf node
        tree[tree_index].isInTree = false;   // The node is in the array, but it is not in the tree yet

        if (ind2 != 0xffff) {
            if (tree[ind1].leaf_symbol.symbol < tree[ind2].leaf_symbol.symbol) {
                tree[tree_index].left = ind1;   // The index of the left child is the index of the node with the smaller symbol
                tree[tree_index].right = ind2;  // The index of the right child is the index of the node with the bigger symbol

            } else {
                tree[tree_index].left = ind2;   // The index of the left child is the index of the node with the smaller symbol
                tree[tree_index].right = ind1;  // The index of the right child is the index of the node with the bigger symbol
            }

            // put the two nodes in the tree
            tree[ind1].isInTree = true;
            tree[ind2].isInTree = true;
        }

        tree_index++;
    } while (tree[tree_index - 1].leaf_symbol.symbol_length != 0);  // The only node with symbol length 0 is the root node

    // The return value is the index of the root node
    return tree_index - 1;
}