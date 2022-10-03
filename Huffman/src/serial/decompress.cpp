#include <cstring>
#include <iostream>

#include "../structs.h"
#include "../huffman.h"

#include "decompress.h"
#include "../file_utils.h"

#define CHAR_BUFF_SIZE 2048  // The size of the write buffer
#define SYM_BUFF_SIZE 128   // The size of the read buffer single element

//#define DEBUG_MODE


/**
 * Decodes a buffer of symbol bits using the huffman tree. If the character buffer is full it is written in the
 * decompressed file. The function is inlined because for large files it is called multiple times adding a memory
 * transfer overhead.
 *
 * @param nodes          The huffman tree array
 * @param root_index     The index of the root node of the tree
 * @param node           The pointer to the current node
 * @param buffer         The symbol buffer
 * @param buffer_start   The starting index of the buffer
 * @param buffer_size    The buffer size (length)
 * @param element_size   The number of bits per buffer element
 * @param char_buffer    The character buffer
 * @param c_index        The index of the character buffer
 * @param decompressed   The pointer of the decompressed file
 */
inline void decodeBuffer(HuffmanNode *nodes, uint16_t root_index, HuffmanNode *node, uint128_t *buffer, uint16_t buffer_start,
                         uint16_t buffer_size, uint8_t element_size, uint8_t *char_buffer, uint32_t *c_index,
                         FILE *decompressed){

    uint128_t bit = 0;  // The current bit of the symbol

    uint128_t bit_mask = 1;      // The mask that gives the next bit
    bit_mask = bit_mask << (SYM_BUFF_SIZE - 1);  // The mask starts from the beginning of the buffer 10000000000....

    // iterate the buffer from start to finish
    for (uint16_t i = buffer_start; i < buffer_size; ++i) {
        for (uint8_t j = 0; j < element_size; ++j) {
            bit = buffer[i] & bit_mask;  // get the next bit of the symbol
            bit_mask = bit_mask >> 1;  // Advance the mask to the next bit

            if (bit == 0) { // If the bit is 0 take the left path of the tree
                *node = nodes[node->left];

            } else { // Else the bit is 1 and we take the right path of the tree
                *node = nodes[node->right];
            }

            // If the new node is a leaf node, then the symbol is complete
            if (node->isLeaf) {
                char_buffer[*c_index] = node->ascii_index;  // store the character in the char buffer
                *c_index += 1;  // increment the char_buffer index

                // If the char buffer is full ...
                if (*c_index == CHAR_BUFF_SIZE) {
                    // ...write the characters to the decompressed file ...
                    fwrite(char_buffer, sizeof(char_buffer[0]), *c_index, decompressed);

                    // ... and reset the index
                    *c_index = 0;
                }

                // Go to the start of the tree from the next symbol
                *node = nodes[root_index];
            }
        }

        // reset the bit mask
        bit_mask = 1;
        bit_mask = bit_mask << (SYM_BUFF_SIZE - 1);
    }
}


/**
 * Decompresses a file. The steps to decompress the file are the following:
 *
 *   Step 1: Read the meta data from of the file.
 *
 *           Byte 0:3       The number of the padding bits added to the end of the file (uint32_t)
 *
 *           Byte 4:7       The number of blocks in the file (uint32_t)
 *
 *           Byte 8:9       The block size used to group data (uint16_t)
 *
 *           Byte 10:8457   The huffman table used to compress the file. After every 256 bit symbol the number of bits
 *                          used by the symbol are written as well as an 8 bit number.
 *                          The size of the table is 256 x (256 + 8) bits
 *
 *           Byte 8457:end  The compressed data
 *
 *   Step 2: Create the Huffman tree from the huffman table
 *
 *   Step 3: Decode the symbols to characters and write them to the decompressed file
 *
 * @param filename  The name of the file to be decompressed
 */
void decompressFile(const char *filename){
    // Open the decompressed file in read mode
    FILE *file = openBinaryFile(filename, "rb");

    // Create a new file for decompression
    int len = (int) strlen(filename); // Get the length of the old filename

    char *newFilename = (char *) malloc((len - 1) * sizeof(char));  // Allocate enough space for the new file name

    memcpy(newFilename, filename, sizeof(char) * (len - 5));  // Copy the old name to the new name

    char end[5] = ".dec";  // The string to be appended to the file name

    memcpy(newFilename + len - 5, end, sizeof(char) * 5);  // Append the ending to the file name

    // Create the new file
    FILE *decompressed = openBinaryFile(newFilename, "wb");

    if (decompressed == nullptr) {
        std::cout << "\n\nCould not create file" << std::endl;
        exit(1);
    }

    free(newFilename);  // free the no longer needed memory

    // This is the number of padding bits to the end of the file. The padding bits align the data to bytes.
    uint32_t padding_bits = 0;

    // The number of blocks written to the file
    uint32_t n_blocks = 0;

    // The block size used to group data during the compression
    uint16_t block_size = 0;

#ifdef DEBUG_MODE
    cout << "    Reading metadata..." << endl;
#endif

    // Read the numbers from the file
    fread(&padding_bits, sizeof(padding_bits), 1, file);
    fread(&n_blocks, sizeof(n_blocks), 1, file);
    fread(&block_size, sizeof(block_size), 1, file);

    // Retrieve the huffman info
    ASCIIHuffman huffman;

    for (Symbol &symbol : huffman.symbols) {
        // Read all the symbols from the file
        fread(&symbol.symbol, sizeof(huffman.symbols[0].symbol), 1, file);
        fread(&symbol.symbol_length, sizeof(huffman.symbols[0].symbol_length), 1, file);
    }

#ifdef DEBUG_MODE
    cout << "\n\nPadding bits: " << padding_bits << ", Blocks: " << n_blocks << ", block size: " << block_size << endl;
    cout << "\nRead Huffman symbols:" << endl;

    // Print the symbol array
    for (auto & symbol : huffman.symbols) {
        cout << "len: " << unsigned(symbol.symbol_length) << ", sym: " << hex << symbol.symbol << dec << endl;
    }
#endif

    // The huffman tree
    HuffmanNode nodes[511];
    uint16_t root_index = huffmanFromArray(&huffman, nodes);

#ifdef DEBUG_MODE
    cout << "Created tree:" << endl;
    printTree(nodes, root_index);
#endif

    /*
     * Start reading the file from start to finish. Reading one bit at a time and navigating the tree until a leaf node
     * is reached.
     */
    uint16_t buffer_size = block_size / SYM_BUFF_SIZE;

    /*
     * The buffer holds the data to be written to the file. Once the buffer is full the data are written to the file
     * and the buffer is overwritten with the next part of data. The process repeats until the end
     */
    auto *buffer = (uint128_t *) calloc(buffer_size, sizeof(uint128_t));


    uint8_t char_buffer[CHAR_BUFF_SIZE];    // The decompressed character
    uint32_t c_index = 0;         // The character buffer index

    HuffmanNode node = nodes[root_index];  // The current node of the tree.

    // If the file has only one block skip to the final block handling
    if (n_blocks > 1) {
        // For all the blocks in the file except the last one...
        do {
            // read the symbol bits from the compressed file
            fread(&buffer[0], sizeof(buffer[0].lower()), buffer_size * 2, file);

            // decode the buffer
            decodeBuffer(nodes, root_index, &node, buffer, 0, buffer_size, SYM_BUFF_SIZE,
                         char_buffer, &c_index, decompressed);

            // Update the remaining blocks number
            n_blocks--;

        } while (n_blocks != 1);
    }

    // The last block may contain padding bits that shouldn't be interpreted as symbols
    fread(&buffer[0], sizeof(buffer[0].lower()), buffer_size * 2, file);  // Read the last block

    // Get the number of full elements of the buffer
    buffer_size -= padding_bits / SYM_BUFF_SIZE;

    // decode the buffer except the last buffer element
    decodeBuffer(nodes, root_index, &node, buffer, 0, buffer_size - 1, SYM_BUFF_SIZE,
                 char_buffer, &c_index, decompressed);

    // The last element that contains symbol bits may contain some padding bits also
    // The number of symbol bits on the last element of the buffer
    uint8_t useful_bits = SYM_BUFF_SIZE - padding_bits % SYM_BUFF_SIZE;

    // decode the last element of the buffer
    decodeBuffer(nodes, root_index, &node, buffer, buffer_size - 1, buffer_size, useful_bits,
                 char_buffer, &c_index, decompressed);

    // Write the remaining chars
    fwrite(char_buffer, sizeof(char_buffer[0]), c_index, decompressed);

    fclose(decompressed);
    fclose(file);
}