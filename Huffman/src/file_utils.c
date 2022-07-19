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
 * Takes the file to be compressed reads the bits and creates a new compressed file.
 *
 * @param file          The original file
 * @param filename      The filename of the compressed file
 * @param asciiHuffman  The huffman struct that contains the information for the compression
 */
void compressFile(FILE *file, char *filename, ASCIIHuffman *asciiHuffman){
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

    // Write the huffman table to the beginning of the file
    fwrite(asciiHuffman->symbols, sizeof(uint16_t), 256, compressed);

    // Data must be written in bytes. Since every symbol is not a complete byte the symbols must be buffered to bytes.
    uint8_t buffer_1;
    uint8_t buffer_2;

    // Start reading from the file and converting chars to symbols
    // TODO write symbols to file


    free(newFilename);
    fclose(compressed);
}