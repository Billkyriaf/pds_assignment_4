#include "char_frequency.h"
#include "../file_utils.h"

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
void charFrequency(const char *filename, ASCIIHuffman *huffman) {
    FILE *file = openBinaryFile(filename, "rb");

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