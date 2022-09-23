#include <cstdlib>
#include <cstring>
#include <iostream>

#include "file_utils_pthread.h"
#include "huffman_tree_pthread.h"

#define CHAR_BUFF_SIZE 2048  // The size of the write buffer
#define SYM_BUFF_SIZE 128   // The size of the read buffer single element

//#define DEBUG_MODE

using namespace std;


/**
 * Opens a file in binary format for reading
 *
 * @param filename The file name
 * @param mode     The mode to open the file
 * @return  The FILE pointer created
 */
FILE *openBinaryFile(const char *filename, const char *mode) {
    FILE *file = fopen(filename, mode);

    if (file == nullptr) {
        printf("File not found...\n");
        exit(-1);
    }

    return file;
}


/**
 * A symbol is inserted in the buffer. If the buffer is full after the insertion the buffer is written to the compressed
 * file.
 *
 * @param buffer         The buffer that is written to the file. The buffer is 128 x n array
 * @param buff_index     The index to the buffer
 * @param write_index    The index of the bit for the 128 bit buffer element
 * @param compressed     The file compressed pointer
 * @param symbol_length  The length of the symbol to insert to the buffer
 * @param symbol         The symbol to be inserted in the buffer
 * @param bufferSize     The size of the buffer
 */
void insertToBuffer(uint128_t *buffer, int *buff_index, uint8_t *write_index, uint32_t *nBlocks, FILE *compressed,
                    uint8_t symbol_length, uint256_t symbol, uint16_t bufferSize) {

    buffer[*buff_index] = buffer[*buff_index] << symbol_length;  // make room for the new symbol

    // Keep only the 128 LSBs of the symbol and append it to the buffer
    buffer[*buff_index] += symbol.lower();

    if (*write_index + 1 - symbol_length == 0) {  // if the symbol fits exactly...
        *write_index = SYM_BUFF_SIZE - 1;  // ... reset the write_index ...
        *buff_index += 1;  // ... and increment the buffer index

        // If the buffer is full...
        if (*buff_index == bufferSize){
            //... write the buffer to the file...
            fwrite(buffer, sizeof(buffer[0]), bufferSize, compressed);

            *nBlocks += 1;  // increment the number of blocks
            *buff_index = 0;  // ... and reset the index

            // reset the buffer
            for (int i = 0; i < bufferSize; ++i) {
                buffer[i] = 0;
            }
        }

    } else {  // else if there is still space in the symbol
        *write_index -= symbol_length;  // Update the write index
    }
}


/**
 * The worst case regarding symbol length is 32 bytes. This is a rare and very specific case. To conserve as much
 * space as possible we store the symbols using the minimum number of bytes required. After every symbol the length
 * of it is written. Before the huffman table the number of bytes per symbols is also written in the file
 *
 * @param file     The compressed file pointer
 * @param huffman  The huffman struct containing all the symbols
 */
void writeHuffmanToFile(FILE *file, ASCIIHuffman *huffman, uint16_t *meta_data_size) {

    // Write the symbols with their lengths in the file
    for (Symbol &symbol : huffman->symbols) {
        // Write the huffman table to the beginning of the file
        fwrite(&symbol.symbol, sizeof(huffman->symbols[0].symbol), 1, file);
        *meta_data_size += sizeof(huffman->symbols[0].symbol);

        // Write the length of the symbol to the file
        fwrite(&symbol.symbol_length, sizeof(symbol.symbol_length), 1, file);
        *meta_data_size += sizeof(symbol.symbol_length);
    }
}


typedef struct compress_args{
    int t_id;                 /// The id of the thread
    char const *file;         /// The file to be compressed
    char const *output_file;  /// The compressed file
    ASCIIHuffman *huffman;    /// The huffman struct containing the symbols

    uint64_t start_byte;      /// The thread reads from this byte (inclusive)
    uint64_t end_byte;        /// The thread reads up to this byte (exclusive)

    uint64_t compressed_start_byte;    /// The thread starts writing from this byte (inclusive)
    uint64_t compressed_end_byte;      /// The thread stops writing up to this byte (exclusive)

    uint32_t *number_of_blocks;        /// The number of blocks that the thread writes to the file
    uint32_t *number_of_padding;       /// The number of padding bits that the thread writes to the end of the section

    uint16_t buffer_size;              /// The size of the buffer in bytes
} CompressArgs;


/**
 * The thread function that compresses the file. Every thread has to compress a part of the file
 * @param args  The arguments of the thread (CompressArgs)
 * @return nullptr
 */
void *compressFileRunnable(void *args) {
    // Cast the arguments to the correct type
    auto arguments = (CompressArgs *) args;

    // Extract some of the arguments for cleaner looking code
    ASCIIHuffman *huffman = arguments->huffman;
    uint32_t *n_blocks = arguments->number_of_blocks;
    uint32_t *n_padding_bits = arguments->number_of_padding;
    uint16_t  buffer_size = arguments->buffer_size;


    // Open the file to be compressed
    FILE *file = openBinaryFile(arguments->file, "rb");

    if (file == nullptr) {
        printf("File not found...\n");
        exit(-1);
    }

    // Open the compressed file
    FILE *compressed = openBinaryFile(arguments->output_file, "rb+");

    if (compressed == nullptr) {
        printf("File not found...\n");
        exit(-1);
    }

    // Seek the start of the file to the starting byte
    fseek(file, (long int)arguments->start_byte, SEEK_SET);

    // Seek the start of the compressed file to the starting byte
    fseek(compressed, (long int)arguments->compressed_start_byte, SEEK_SET);

    // Start compressing the file

    // The buffer holds the data to be written to the file. Once the buffer is full the data are written to the file
    // and the buffer is overwritten with the next part of data. The process repeats until the end
    auto *buffer = (uint128_t *)calloc(buffer_size, sizeof(uint128_t));

    // Start reading from the file and converting chars to symbols
    uint8_t c;  // The character read from the file

    uint256_t symbol = 0;  // The symbol along with the symbol length of every char
    uint8_t symbol_length = 0;  // The symbol length
    uint8_t write_index = SYM_BUFF_SIZE - 1;  // The index of the start point of the symbol in the buffer

    // The index to the buffer
    int buff_index = 0;

    // Read from the file byte by byte
    for (long unsigned int i = 0; i < arguments->end_byte; ++i) {
        fread(&c, sizeof(c), 1, file);  // Read byte from the file

        symbol = huffman->symbols[c].symbol;  // The symbol of the read char
        symbol_length = huffman->symbols[c].symbol_length;  // The number of bits of the symbol

        if (write_index + 1 - symbol_length < 0) {  // If the buffer can't fit the symbol
            // The buffer can fit write_index + 1 bits of the symbol

            uint256_t mask = (1 << (symbol_length - write_index - 1)) - 1;

            // Split the symbol
            uint256_t remaining_symbol = symbol & mask;  // keep the remaining symbol
            uint8_t remaining_length = symbol_length - write_index - 1;

            symbol = symbol & ~mask;  // The part of the symbol that fits
            symbol = symbol >> remaining_length;  // realign the symbol
            symbol_length = write_index + 1;  // The length of the symbol that fits

            // append the symbol to the buffer and if the buffer is full write to the file
            insertToBuffer(buffer, &buff_index, &write_index, n_blocks, compressed, symbol_length, symbol, buffer_size);

            // append the remaining symbol to the buffer and if the buffer is full write to the file
            insertToBuffer(buffer, &buff_index, &write_index, n_blocks, compressed, remaining_length, remaining_symbol,
                           buffer_size);

        } else {  // else if the symbol fits in the buffer

            // append to the buffer and if the buffer is full write to the file
            insertToBuffer(buffer, &buff_index, &write_index, n_blocks, compressed, symbol_length, symbol, buffer_size);
        }
    }

    // The final buffer may not be full. In that case the rest of the block bits will be 0 and will be counted as padding

    // if the buffer index is not 0 or the write index is not 7 the last buffer was not full
    if (buff_index != 0 || write_index != SYM_BUFF_SIZE - 1) {

        // Update the number of padding bits. Every buffer element is SYM_BUFF_SIZE bits
        *n_padding_bits += SYM_BUFF_SIZE * (buffer_size - buff_index - 1);

        // zero the remaining bits in the buffer[buff_index]
        if (write_index != SYM_BUFF_SIZE - 1) {

            // Align the last SYM_BUFF_SIZE bits
            buffer[buff_index] = buffer[buff_index] << (write_index + 1);

            *n_padding_bits += write_index + 1;  // add the final padding bits
        }

        // write the buffer to the file
        fwrite(buffer, sizeof(buffer[0]), buffer_size, compressed);

        *n_blocks += 1;  // Update the number of blocks
    }

    // close the files and free the memory
    fclose(file);
    fclose(compressed);
    free(buffer);

    pthread_exit(nullptr);
}


/**
 * Takes the file to be compressed reads the bits and creates a new compressed file. The compressed file has the
 * following format:
 *      Byte 0         The number of sections in the file (uint8_t)  (In this example 3)
 *      Byte 1:4       The number of the padding bits added to the end of the first section (uint32_t)
 *      Byte 5:8       The number of the padding bits added to the end of the second section (uint32_t)
 *      Byte 9:12      The number of the padding bits added to the end of the third section (uint32_t)
 *      .
 *      .
 *      .
 *      Byte 13:16     The number of blocks in the first section (uint32_t)
 *      Byte 17:20     The number of blocks in the second section (uint32_t)
 *      Byte 21:24     The number of blocks in the third section (uint32_t)
 *      .
 *      .
 *      .
 *      Byte 25:26     The block size used to group data (uint16_t)
 *      Byte 27:8483   The huffman table used to compress the file. After every 256 bit symbol the number of bits used
 *                     by the symbol are written as well as an 8 bit number. The size of the table is 256 x (256 + 8) bits
 *      Byte 8484:end  The compressed data
 *
 * @param file       The original file
 * @param filename   The filename of the compressed file
 * @param huffman    The huffman struct that contains the information for the compression
 * @param block_size The size in bytes of the data that every write operation writes to the file. (must be power of 2)
 */
void compressFile(const char *filename, ASCIIHuffman *huffman, uint16_t block_size) {

    // Create the new file name
    int len = (int) strlen(filename); // Get the length of the old filename
    char *newFilename = (char *) malloc((len + 6) * sizeof(char));  // Allocate enough space for the new file name

    memcpy(newFilename, filename, sizeof(char) * len);  // Copy the old name to the new name

    char end[6] = ".huff";  // The string to be appended to the file name

    memcpy(newFilename + len, end, sizeof(char) * 6);  // Append the ending to the file name

    // Create the new file
    FILE *compressed = openBinaryFile(newFilename, "wb");

    if (compressed == nullptr) {
        cout << "\n\nCould not create file" << endl;
        exit(1);
    }

    uint16_t meta_data_size = 0;  // The size of the meta data in bytes

    // STEP 1 - Write the number of sections
    uint8_t n_sections = N_THREADS;  // The number of sections the file is divided to
    fwrite(&n_sections, sizeof(n_sections), 1, compressed);  // Write the number of sections to the file
    meta_data_size += sizeof(n_sections);  // Update the meta data size

    // STEP 2 - Write the number of padding bits of each section (updated in the end)
    uint32_t section_padding[N_THREADS] = {0};  // The number of padding bits of each section (initially 0)
    fwrite(&section_padding, sizeof(section_padding[0]), N_THREADS, compressed);  // Write the padding bits to the file
    meta_data_size += sizeof(section_padding[0]) * N_THREADS;  // Update the meta data size

    // STEP 3 - Write the number of blocks of each section (updated in the end)
    uint32_t n_blocks[N_THREADS] = {0};  // The number of blocks written to the file
    fwrite(&n_blocks, sizeof(n_blocks[0]), N_THREADS, compressed);  // Write the number of blocks.
    meta_data_size += sizeof(n_blocks) * N_THREADS;  // Update the meta data size

    // STEP 4 - Write the block size used to group data to the compressed file.
    fwrite(&block_size, sizeof(block_size), 1, compressed);
    meta_data_size += sizeof(block_size);  // Update the meta data size

    // STEP 5 - Write the huffman table to the beginning of the file
    writeHuffmanToFile(compressed, huffman, &meta_data_size);

    uint16_t buffer_size = block_size / SYM_BUFF_SIZE;

    CompressArgs args[N_THREADS];  // The arguments for the threads

    // Create the arguments of every thread
    for (int i = 0; i < N_THREADS; i++) {
        args[i].t_id = i;  // Set the thread id

        args[i].file = filename;  // The name of the file to be compressed
        args[i].output_file = newFilename;  // The name of the compressed file

        args[i].huffman = huffman;  // The huffman struct containing the symbols

        args[i].start_byte = 0;
        args[i].end_byte = 0;
        args[i].compressed_start_byte = 0;
        args[i].compressed_end_byte = 0;

        for (int j = 0; j < 255; ++j) {
            // Find the number of bytes each thread has to compress
            args[i].end_byte += huffman->frequencies[i][j];

            // find the number of compressed bits that the thread has to write
            args[i].compressed_end_byte += huffman->frequencies[i][j] * huffman->symbols[j].symbol_length;
        }

        // if the number of bits don't align to the block size
        if (args->compressed_end_byte % (block_size * 8) != 0) {
            // This is the number of blocks required
            args->compressed_end_byte = (args->compressed_end_byte / (block_size * 8)) + 1;

            // Convert the number of blocks to bytes
            args->compressed_end_byte *= block_size;
        }
        else {
            // This is the number of blocks required
            args->compressed_end_byte /= (block_size * 8);

            // Convert the number of blocks to bytes
            args->compressed_end_byte *= block_size;
        }

        // Calculate the start and end bytes
        if (i == 0) {
            args[i].start_byte = 0;  // The first thread starts from the beginning of the file

            // The first thread starts writing from the end of the meta data
            args[i].compressed_start_byte = meta_data_size;

            // The first thread stops writing at the end of the meta data + the number of bytes it has to write
            args[i].compressed_end_byte += meta_data_size;

        } else {
            args[i].start_byte = args[i - 1].end_byte;  // The start byte is the end byte of the previous thread
            args[i].end_byte += args[i].start_byte;  // The end byte is the calculated end byte + the start byte

            // The start byte of the thread is the end byte of the previous thread
            args[i].compressed_start_byte = args[i - 1].compressed_end_byte;

            // The end byte of the thread is the calculated end byte + the start byte
            args[i].compressed_end_byte += args[i].compressed_start_byte;
        }

        args[i].number_of_blocks = &n_blocks[i];  // The number of blocks that the thread writes to the file
        args[i].number_of_padding = &section_padding[i];  // The number of padding bits that the thread writes to the end of it's section
        args[i].buffer_size = buffer_size;  // The size of the buffer
    }

    // Create the threads attributes
    pthread_attr_t attributes[N_THREADS];
    for (auto & attribute : attributes) {
        pthread_attr_init(&attribute);
    }

    pthread_t threads[N_THREADS];

    // Create the threads
    for (int i = 0; i < N_THREADS; i++) {
        args[i].t_id = i;
        pthread_create(&threads[i], &attributes[i], compressFileRunnable, &args[i]);
    }

    // Join the threads
    for (unsigned long thread : threads) {
        pthread_join(thread, nullptr);
    }

    // update the number of padding bits and the number of blocks written tho the compressed file
    fseek(compressed, 1, SEEK_SET);  // Seek to the beginning of the padding bits

    // Write the number of padding bits of every section to the meta data.
    fwrite(&section_padding, sizeof(section_padding[0]), N_THREADS, compressed);

    // Write the number of blocks of every section to the meta data.
    fwrite(&n_blocks, sizeof(n_blocks[0]), N_THREADS, compressed);

    // Close the file free memory and destroy the attributes
    free(newFilename);
    fclose(compressed);

    for (auto & attribute : attributes) {
        pthread_attr_destroy(&attribute);
    }
}


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
        cout << "\n\nCould not create file" << endl;
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