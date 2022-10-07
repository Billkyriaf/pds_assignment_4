#include <cstring>
#include <iostream>
#include <cilk/cilk.h>

#include "../structs.h"
#include "../huffman.h"
#include "../file_utils.h"
#include "decompress_cilk.h"


#define CHAR_BUFF_SIZE 2048  // The size of the write buffer
#define SYM_BUFF_SIZE 128   // The size of the read buffer single element

//#define DEBUG_MODE

using namespace std;

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

            } else { // Else the bit is 1, and we take the right path of the tree
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

typedef struct decompress_job_args{
    int t_id;                 /// The id of the thread
    char const *file;         /// The file to be decompressed
    char const *output_file;  /// The decompressed file
    ASCIIHuffman *huffman;    /// The huffman struct containing the symbols

    uint64_t start_byte;      /// The thread reads from this byte (inclusive)
    uint64_t end_byte;        /// The thread reads up to this byte (exclusive)

    uint64_t decompressed_start_byte;  /// The thread starts writing from this byte (inclusive)
    uint64_t decompressed_end_byte;    /// The thread stops writing up to this byte (exclusive)

    uint32_t number_of_blocks;        /// The number of blocks that the thread has to decompress
    uint32_t number_of_padding;       /// The number of padding bits that the thread has to the end of it's section

    uint16_t buffer_size;              /// The size of the buffer in bytes

} DecompressJobArgs;


/**
 * The thread function that decompresses the file. Every thread has to decompress a part of the file
 * @param args  The arguments of the thread (DecompressArgs)
 * @return nullptr
 */
void decompressFileJob(DecompressJobArgs *decompress_args){

    // The huffman tree
    HuffmanNode nodes[511];
    // Create the nodes of the tree
    uint16_t root_index = huffmanFromArray(decompress_args->huffman, nodes);

    // Open the files
    FILE *input_file = fopen(decompress_args->file, "rb");
    FILE *decompressed = fopen(decompress_args->output_file, "rb+");

    // Seek the starting position of the files
    fseek(input_file, (long int)decompress_args->start_byte, SEEK_SET);
    fseek(decompressed, (long int)decompress_args->decompressed_start_byte, SEEK_SET);

    /*
     * The buffer holds the data to be written to the input_file. Once the buffer is full the data are written to the input_file
     * and the buffer is overwritten with the next part of data. The process repeats until the end
     */
    auto *buffer = (uint128_t *) calloc(decompress_args->buffer_size, sizeof(uint128_t));


    uint8_t char_buffer[CHAR_BUFF_SIZE];    // The decompressed character
    uint32_t c_index = 0;         // The character buffer index

    HuffmanNode node = nodes[root_index];  // The current node of the tree.

    // If the input_file has only one block skip to the final block handling
    if (decompress_args->number_of_blocks > 1) {
        // For all the blocks in the input_file except the last one...
        do {
            // read the symbol bits from the compressed input_file
            fread(&buffer[0], sizeof(buffer[0].lower()), decompress_args->buffer_size * 2, input_file);

            // decode the buffer
            decodeBuffer(nodes, root_index, &node, buffer, 0, decompress_args->buffer_size, SYM_BUFF_SIZE,
                         char_buffer, &c_index, decompressed);

            // Update the remaining blocks number
            decompress_args->number_of_blocks--;

        } while (decompress_args->number_of_blocks != 1);
    }

    // The last block may contain padding bits that shouldn't be interpreted as symbols
    fread(&buffer[0], sizeof(buffer[0].lower()), decompress_args->buffer_size * 2, input_file);  // Read the last block

    // Get the number of full elements of the buffer
    decompress_args->buffer_size -= decompress_args->number_of_padding / SYM_BUFF_SIZE;

    // decode the buffer except the last buffer element
    decodeBuffer(nodes, root_index, &node, buffer, 0, decompress_args->buffer_size - 1, SYM_BUFF_SIZE,
                 char_buffer, &c_index, decompressed);

    // The last element that contains symbol bits may contain some padding bits also
    // The number of symbol bits on the last element of the buffer
    uint8_t useful_bits = SYM_BUFF_SIZE - decompress_args->number_of_padding % SYM_BUFF_SIZE;

    // decode the last element of the buffer
    decodeBuffer(nodes, root_index, &node, buffer, decompress_args->buffer_size - 1, decompress_args->buffer_size, useful_bits,
                 char_buffer, &c_index, decompressed);

    // Write the remaining chars
    fwrite(char_buffer, sizeof(char_buffer[0]), c_index, decompressed);

    free(buffer);
    fclose(input_file);
    fclose(decompressed);
}

/**
 * Decompresses a file. The steps to decompress the file are the following:
 *
 *   Step 1: Read the meta data from of the file.
 *
 *      Byte 0         The number of sections in the file (uint8_t)  (In this example 3)
 *      Byte 1:8       The number of decompressed characters of the first section (uint64_t)
 *      Byte 9:16      The number of decompressed characters of the second section (uint64_t)
 *      Byte 17:24     The number of decompressed characters of the third section (uint64_t)
 *      .
 *      .
 *      .
 *      Byte 45:28     The number of the padding bits added to the end of the first section (uint32_t)
 *      Byte 29:32     The number of the padding bits added to the end of the second section (uint32_t)
 *      Byte 33:36     The number of the padding bits added to the end of the third section (uint32_t)
 *      .
 *      .
 *      .
 *      Byte 37:40     The number of blocks in the first section (uint32_t)
 *      Byte 41:44     The number of blocks in the second section (uint32_t)
 *      Byte 45:48     The number of blocks in the third section (uint32_t)
 *      .
 *      .
 *      .
 *      Byte 49:50     The block size used to group data (uint16_t)
 *      Byte 51:8507   The huffman table used to compress the file. After every 256 bit symbol the number of bits used
 *                     by the symbol are written as well as an 8 bit number. The size of the table is 256 x (256 + 8) bits
 *      Byte 8508:end  The compressed data
 *
 *   Step 2: Create the Huffman tree from the huffman table
 *
 *   Step 3: Decode the symbols to characters and write them to the decompressed file
 *
 * @param filename  The name of the file to be decompressed
 */
void decompressFile(const char *filename){
    // Create a new input_file for decompression
    int len = (int) strlen(filename); // Get the length of the old filename

    char *newFilename = (char *) malloc((len) * sizeof(char));  // Allocate enough space for the new input_file name

    memcpy(newFilename, filename, sizeof(char) * (len - 5));  // Copy the old name to the new name

    char end[] = ".dec";  // The string to be appended to the input_file name

    memcpy(newFilename + len - 5, end, sizeof(char) * 5);  // Append the ending to the input_file name


    // Open the decompressed input_file in read mode
    FILE *input_file = openBinaryFile(filename, "rb");

    // Start reading the metadata from the input file

#ifdef DEBUG_MODE
    cout << "    Reading metadata..." << endl;
#endif
    uint16_t  meta_data_size = 0;  // The size of the metadata in bytes

    // STEP 1 - Read the number of sections
    uint8_t n_sections;  // The number of sections is yhe number of threads used to compress the file
    fread(&n_sections, sizeof(uint8_t), 1, input_file);  // read the number of sections
    meta_data_size += sizeof(uint8_t);  // increment the meta data size

    // STEP 2 - Read the number of characters of each section
    auto *section_sizes = (uint64_t *) malloc(n_sections * sizeof(uint64_t));
    fread(section_sizes, sizeof(uint64_t), n_sections, input_file);
    meta_data_size += sizeof(uint64_t) * n_sections;  // increment the meta data size

    // STEP 3 - Read the number of padding bits of each section
    auto *section_padding = (uint32_t *) malloc(n_sections * sizeof(uint32_t));
    fread(section_padding, sizeof(uint32_t), n_sections, input_file);
    meta_data_size += sizeof(uint32_t) * n_sections;  // increment the meta data size

    // STEP 4 - Read the number of blocks of each section
    auto *n_blocks = (uint32_t *) malloc(n_sections * sizeof(uint32_t));
    fread(n_blocks, sizeof(uint32_t), n_sections, input_file);
    meta_data_size += sizeof(uint32_t) * n_sections;  // increment the meta data size

    // STEP 5 - Read the block size used to group data to the compressed file.
    uint16_t block_size;
    fread(&block_size, sizeof(block_size), 1, input_file);
    meta_data_size += sizeof(block_size);  // increment the meta data size

    // Retrieve the huffman info
    ASCIIHuffman huffman;

    for (Symbol &symbol : huffman.symbols) {
        // Read all the symbols from the input_file
        fread(&symbol.symbol, sizeof(huffman.symbols[0].symbol), 1, input_file);
        meta_data_size += sizeof(huffman.symbols[0].symbol);  // increment the meta data size

        fread(&symbol.symbol_length, sizeof(huffman.symbols[0].symbol_length), 1, input_file);
        meta_data_size += sizeof(huffman.symbols[0].symbol_length);  // increment the meta data size
    }

#ifdef DEBUG_MODE
    cout << "\n\nPadding bits:" << endl;
        for (int i = 0; i < n_sections; ++i) {
            cout << "    Section " << i << ", has " << section_padding[i] << " bits" <<endl;
        }

        cout << "\nNumber of blocks:" << endl;
        for (int i = 0; i < n_sections; ++i) {
            cout << "    Section " << i << ", has " << n_blocks[i] << " blocks" <<endl;
        }

        cout << "\nBlock size: " << block_size << endl;

        cout << "\n\nRead Huffman symbols:" << endl;

        // Print the symbol array
        for (auto & symbol : huffman.symbols) {
            cout << "len: " << unsigned(symbol.symbol_length) << ", sym: " << hex << symbol.symbol << dec << endl;
        }

        cout << "Created tree:" << endl;
        printTree(nodes, root_index);
#endif

    // For the decompression to work in parallel every thread needs to know the limits of the section it is responsible for
    auto *args = (DecompressJobArgs *) malloc(n_sections * sizeof(DecompressJobArgs));

    // Prepare the arguments for the threads
    for (int i = 0; i < n_sections; ++i) {
        args[i].file = filename;
        args[i].output_file = newFilename;

        if (i == 0){
            args[i].start_byte = meta_data_size;
            args[i].decompressed_start_byte = 0;
        } else {
            args[i].start_byte = args[i - 1].end_byte;
            args[i].decompressed_start_byte = args[i - 1].decompressed_end_byte;
        }

        // The block size is in bits, so we need to convert it to bytes
        args[i].end_byte = args[i].start_byte + n_blocks[i] * (block_size / 8);
        args[i].decompressed_end_byte = args[i].decompressed_start_byte + section_sizes[i];

        args[i].number_of_blocks = n_blocks[i];
        args[i].number_of_padding = section_padding[i];

        args[i].buffer_size = block_size / SYM_BUFF_SIZE;

        args[i].huffman = &huffman;
    }

    // Spawn the function
    cilk_for (int i = 0; i < n_sections; ++i) {
        decompressFileJob(&args[i]);
    }

    fclose(input_file);
    free(newFilename);
    free(args);
    free(section_sizes);
    free(section_padding);
    free(n_blocks);
}