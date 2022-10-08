#ifndef COMPRESS_PTH_H
#define COMPRESS_PTH_H

#include "../structs.h"


/**
 * Takes the file to be compressed reads the bits and creates a new compressed file. The compressed file has the
 * following format:
 *      Byte 0         The number of sections in the file (uint8_t)  (In this example 3)
 *      Byte 1:8       The number of characters of the first section (uint64_t)
 *      Byte 9:16      The number of characters of the second section (uint64_t)
 *      Byte 17:24     The number of characters of the third section (uint64_t)
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
 * @param filename             The name of the input file
 * @param compressed_filename  The name of the compressed file
 * @param huffman              The huffman struct that contains the information for the compression
 * @param block_size           The size in bits of the data that every write operation writes to the file. (must be power of 2)
 */
void compressFile(const std::string& filename, const std::string& compressed_filename, ASCIIHuffman *huffman, uint16_t block_size);

#endif
