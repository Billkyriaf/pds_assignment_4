#include <stdlib.h>
#include <string.h>

#include "file_utils.h"

/**
 * Opens a file in binary format for reading
 * @param filename The file name
 * @return  The FILE pointer created
 */
FILE *openBinaryFile(char *filename) {
    FILE *file = fopen(filename, "rb");

    if (file == NULL){
        printf("File not found...\n");
        exit(-1);
    }

    return file;
}

/**
 * Takes the file to be compressed reads the bits and creates a new compressed file. The compressed file has the
 * following format:
 *
 *      Byte 0:1      The number of the padding bits added to the end of the file (uint16_t)
 *      Byte 2:5      The number of blocks in the file (uint32_t)
 *      Byte 6:517    The huffman table used to compress the file  (256 x uint16_t)
 *      Byte 517:end  The compressed data
 *
 * @param file          The original file
 * @param filename      The filename of the compressed file
 * @param asciiHuffman  The huffman struct that contains the information for the compression
 * @param blockSize     The size of the data that every write operation writes to the file
 */
void compressFile(FILE *file, char *filename, ASCIIHuffman *asciiHuffman, int blockSize){

    // Create the new file name
    int len = (int)strlen(filename); // Get the length of the old filename
    char *newFilename = (char *) malloc((len + 6) * sizeof(char));  // Allocate enough space for the new file name

    memccpy(newFilename, filename, sizeof(char), len);  // Copy the old name to the new name

    char end[6] = ".huff\0";  // The string to be appended to the file name

    memccpy(newFilename + len, end, sizeof(char), 6);  // Append the ending to the file name

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


    // The number of blocks written to the file
    uint32_t nBlocks = 0;

    // Write the number of blocks. The actual number will be written in the end of the process
    fwrite(&nBlocks, sizeof(nBlocks), 1, compressed);


    // Write the huffman table to the beginning of the file
    fwrite(asciiHuffman->symbols, sizeof(asciiHuffman->symbols[0]), 256, compressed);

    // Jump back to the beginning of the file. The file is already read one time so the pointer must return to the start
    rewind(file);

    // Data must be written in bytes. Since every symbol is not a complete byte the symbols must be buffered to bytes.
    uint8_t buffer_1;
    uint8_t buffer_2;

    // Start reading from the file and converting chars to symbols
    // TODO write symbols to file


    free(newFilename);
    fclose(compressed);
}