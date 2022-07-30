#include <stdlib.h>
#include <string.h>

#include "file_utils_v2.h"


/**
 * Opens a file in binary format for reading
 * @param filename The file name
 * @return  The FILE pointer created
 */
FILE *openBinaryFile(char *filename) {
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        printf("File not found...\n");
        exit(-1);
    }

    return file;
}


/**
 *
 * @param buffer
 * @param buff_index
 * @param write_index
 * @param compressed
 * @param symbol_length
 * @param symbol
 * @param blockSize
 */
void updateBuffer(uint8_t *buffer, int *buff_index, uint8_t *write_index, uint32_t *nBlocks, FILE *compressed,
                  Symbol symbol, uint16_t blockSize) {


    // symbol_length + 3 is the index of the MSB in the symbol
    if (symbol_length + 3 > *write_index) {
        /*int blockSize
         * If the index of the MSB of the symbol is bigger that the index of the next symbol in the buffer
         * then the symbol must be shifted right so that the MSB of the symbol ends up in the same index as the
         * write_index. After that we can simply add the symbol and the buffer and the symbol is appended to the
         * buffer
         */
        symbol = symbol >> (symbol_length + 3 - *write_index);
    } else {
        /*
         * If the index of the MSB of the symbol is smaller that the index of the next symbol in the buffer
         * then the symbol must be shifted left so that the MSB of the symbol ends up in the same index as the
         * write_index. After that we can simply add the symbol and the buffer and the symbol is appended to the
         * buffer
         */
        symbol = symbol << (*write_index - (symbol_length + 3));
    }

    // Keep only the 8 LSBs of the symbol and append it to the buffer
    buffer[*buff_index] += (uint8_t) symbol;


    if (*write_index + 1 - symbol_length == 0) {  // if the symbol fits exactly...
        *write_index = 7;  // ... reset the write_index ...
        *buff_index += 1;  // ... and increment the buffer index

        // If the buffer is full...
        if (*buff_index == blockSize) {
            //... write the buffer to the file...
            fwrite(buffer, sizeof(buffer[0]), blockSize, compressed);

            *nBlocks += 1;  // increment the number of blocks
            *buff_index = 0;  // ... and reset the index

            // reset the buffer
            for (int i = 0; i < blockSize; ++i) {
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
void writeHuffmanToFile(FILE *file, ASCIIHuffman *huffman) {
    // Write the size of the symbols in the file
    fwrite(&huffman->longest_symbol, sizeof(huffman->longest_symbol), 1, file);

    // Write the symbols with their lengths in the file
    for (int i = 0; i < 256; ++i) {
        // Write the huffman table to the beginning of the file
        fwrite(huffman->symbols[i].symbol, sizeof(huffman->symbols[0].symbol[0]), huffman->longest_symbol, file);

        // Write the length of the symbol to the file
        fwrite(&huffman->symbols[i].symbol_length, sizeof(huffman->symbols[i].symbol_length), 1, file);
    }
}


/**
 * Takes the file to be compressed reads the bits and creates a new compressed file. The compressed file has the
 * following format:
 *
 *      Byte 0:1      The number of the padding bits added to the end of the file (uint16_t)
 *      Byte 2:5      The number of blocks in the file (uint32_t)
 *      Byte 6:7      The block size used to group data (uint16_t)
 *      Byte 8        The number of bytes per huffman symbol (uint8_t)
 *      Byte 9:...    The huffman table used to compress the file. The symbols have variable length so after every
 *                    symbol the length of it is written (uint8_t). All the symbols written in the file are the same
 *                    number of bytes but this number is not known beforehand (This number is fount in the 8th byte of
 *                    the file). The total number of symbols is 256
 *      Byte ...:end  The compressed data
 *
 * @param file          The original file
 * @param filename      The filename of the compressed file
 * @param asciiHuffman  The huffman struct that contains the information for the compression
 * @param blockSize     The size in bytes of the data that every write operation writes to the file. (must be power of 2)
 */
void compressFile(FILE *file, char *filename, ASCIIHuffman *asciiHuffman, uint16_t blockSize) {

    // Create the new file name
    int len = (int) strlen(filename); // Get the length of the old filename
    char *newFilename = (char *) malloc((len + 6) * sizeof(char));  // Allocate enough space for the new file name

    memcpy(newFilename, filename, sizeof(char) * len);  // Copy the old name to the new name

    char end[6] = ".huff\0";  // The string to be appended to the file name

    memcpy(newFilename + len, end, sizeof(char) * 6);  // Append the ending to the file name

    // Create the new file
    FILE *compressed = fopen(newFilename, "wb");

    if (compressed == NULL) {
        printf("\n\nCould not create file\n");
        exit(1);
    }


    // This is the number of padding bits to the end of the file. The padding bits align the data to bytes.
    uint16_t nPaddingBits = 0;

    // Write the number of padding bits to the compressed file. This number is 0, and it will be updated in the end
    fwrite(&nPaddingBits, sizeof(nPaddingBits), 1, compressed);


    // The number of blocks written to the file.
    uint32_t nBlocks = 0;

    // Write the number of blocks. The actual number will be written in the end of the process
    fwrite(&nBlocks, sizeof(nBlocks), 1, compressed);

    // Write the block size used to group data to the compressed file.
    fwrite(&blockSize, sizeof(blockSize), 1, compressed);

    // Write the huffman table to the beginning of the file
    writeHuffmanToFile(compressed, asciiHuffman);

    // The max block size is 4KB // TODO come up with a number that I did not pull out of my butt
    if (blockSize > 4096)
        blockSize = 4096;

    // The buffer holds the data to be written to the file. Once the buffer is full the data are written to the file
    // and the buffer is overwritten with the next part of data. The process repeats until the end
    uint8_t *buffer = (uint8_t *) calloc(blockSize, sizeof(uint8_t));

    // Start reading from the file and converting chars to symbols
    uint8_t c;  // The character read from the file

    fseek(file, 0, SEEK_END);  // Jump to the end of the file
    long int file_len = ftell(file);  // Get the current byte offset in the file

    rewind(file);  // Jump back to the beginning of the file

    Symbol symbol;  // The symbol along with the symbol length of every char
    uint8_t symbol_length;  // The symbol length
    uint8_t write_index = 7;  // The index of the start point of the symbol in the buffer

    // The index to the buffer
    int buff_index = 0;

    // Read from the file byte by byte
    for (int i = 0; i < file_len; ++i) {
        fread(&c, sizeof(c), 1, file);  // Read from the file

        symbol = asciiHuffman->symbols[c];  // The symbol of the read char
        symbol_length = symbol.symbol_length;  // The number of bits of the symbol

        if (write_index + 1 - symbol_length < 0) {  // If the buffer can't fit the symbol
            // The buffer can fit write_index + 1 bits of the symbol
            uint16_t mask = (1 << (symbol_length - write_index + 3)) - 1;

            // Split the symbol
            uint16_t remaining_symbol = symbol & mask;  // keep the remaining symbol
            uint8_t remaining_length = symbol_length - write_index - 1;

            symbol = symbol & ~mask;  // The part of the symbol that fits
            symbol = symbol >> remaining_length;  // realign the symbol
            symbol_length = write_index + 1;  // The length of the symbol that fits

            // append the symbol to the buffer and if the buffer is full write to the file
            updateBuffer(buffer, &buff_index, &write_index, &nBlocks, compressed, symbol_length, symbol, blockSize);

            // append the remaining symbol to the buffer and if the buffer is full write to the file
            updateBuffer(buffer, &buff_index, &write_index, &nBlocks, compressed, remaining_length, remaining_symbol,
                         blockSize);

        } else {  // else if the symbol fits in the buffer
            // append to the buffer and if the buffer is full write to the file
            updateBuffer(buffer, &buff_index, &write_index, &nBlocks, compressed, symbol, blockSize);
        }
    }

    // The final buffer may not be full. In that case the rest of the block bits will be 0 and will be counted as padding

    // if the buffer index is not 0 or the write index is not 7 the last buffer was not full
    if (buff_index != 0 || write_index != 7) {
        nPaddingBits +=
                8 * (blockSize - buff_index);  // Update the number of padding bits. Every buffer element is 8 bits

        // zero the remaining bits in the buffer[buff_index]
        if (write_index != 7) {
            uint8_t mask = (1 << (7 - write_index)) - 1;
            mask = mask << (write_index + 1);

            buffer[buff_index] = buffer[buff_index] & mask;

            nPaddingBits += 7 - write_index;  // add the final padding bits
        }

        // write the buffer to the file
        fwrite(buffer, sizeof(buffer[0]), blockSize, compressed);

        nBlocks++;
    }

    // update the number of padding bits and the number of blocks written tho the compressed file
    rewind(compressed);

    // Write the number of padding bits to the compressed file.
    fwrite(&nPaddingBits, sizeof(nPaddingBits), 1, compressed);

    // Write the number of blocks.
    fwrite(&nBlocks, sizeof(nBlocks), 1, compressed);

    free(newFilename);
    free(buffer);
    fclose(compressed);
}

/**
 * Decompresses a file
 * @param filename  The name of the file to be decompressed
 */
void decompressFile(char *filename) {
    // Open the decompressed file in read mode
    FILE *file = fopen(filename, "rb");

    // Create a new file for decompression
    int len = (int) strlen(filename); // Get the length of the old filename

    char *newFilename = (char *) malloc((len - 1) * sizeof(char));  // Allocate enough space for the new file name

    memcpy(newFilename, filename, sizeof(char) * (len - 5));  // Copy the old name to the new name

    char end[5] = ".dec\0";  // The string to be appended to the file name

    memcpy(newFilename + len - 5, end, sizeof(char) * 5);  // Append the ending to the file name

    // Create the new file
    FILE *decompressed = fopen(newFilename, "wb");

    if (decompressed == NULL) {
        printf("\n\nCould not create file\n");
        exit(1);
    }

    free(newFilename);  // free the no longer needed memory


    // Start decompressing the file

    /*
     * 1. Extract basic data (number of padding bits number of blocks and huffman tree)
     * 2. Figure a way to decompress the file
     */

    // This is the number of padding bits to the end of the file. The padding bits align the data to bytes.
    uint16_t nPaddingBits = 0;

    // The number of blocks written to the file
    uint32_t nBlocks = 0;

    // The block size used to group data during the compression
    uint16_t blockSize = 0;

    // Read the numbers from the file
    fread(&nPaddingBits, sizeof(nPaddingBits), 1, file);
    fread(&nBlocks, sizeof(nBlocks), 1, file);
    fread(&blockSize, sizeof(blockSize), 1, file);

    // Retrieve the huffman info
    ASCIIHuffman huffman;

    // Read all the symbols from the file
    fread(&huffman.symbols, sizeof(huffman.symbols[0]), 256, file);


    //********************************
    printf("\n\nPadding bits: %u, Blocks: %u, block size: %u.\n Read Huffman symbols: \n", nPaddingBits, nBlocks,
           blockSize);

    // Print the symbol array
    for (int i = 0; i < 255; ++i) {
        printf("%x, ", huffman.symbols[i]);
    }

    printf("%x\n", huffman.symbols[255]);
    //********************************

    fclose(decompressed);
    fclose(file);
}