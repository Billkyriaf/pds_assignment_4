#ifndef COMPRESS_H
#define COMPRESS_H

#include "../structs.h"

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
 *      Byte 27:8474   The huffman table used to compress the file. After every 256 bit symbol the number of bits used
 *                     by the symbol are written as well as an 8 bit number. The size of the table is 256 x (256 + 8) bits
 *      Byte 8475:end  The compressed data
 *
 * @param file       The original file
 * @param filename   The filename of the compressed file
 * @param huffman    The huffman struct that contains the information for the compression
 * @param block_size The size in bits of the data that every write operation writes to the file. (must be power of 2)
 */
void compressFile(const std::string& filename, const std::string& compressed_filename, ASCIIHuffman *huffman, uint16_t block_size);

#endif
