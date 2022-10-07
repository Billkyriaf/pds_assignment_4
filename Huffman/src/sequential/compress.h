#ifndef COMPRESS_H
#define COMPRESS_H

#include "../structs.h"

/**
 * Takes the file to be compressed reads the bits and creates a new compressed file. The compressed file has the
 * following format:
 *
 *      Byte 0:3       The number of the padding bits added to the end of the file (uint32_t)
 *      Byte 4:7       The number of blocks in the file (uint32_t)
 *      Byte 8:9       The block size used to group data (uint16_t)
 *      Byte 10:8457   The huffman table used to compress the file. After every 256 bit symbol the number of bits used
 *                     by the symbol are written as well as an 8 bit number. The size of the table is 256 x (256 + 8) bits
 *      Byte 8457:end  The compressed data
 *
 * @param filename         The name of the file to be compressed
 * @param output_filename  The name of the compressed file
 * @param huffman          The huffman struct that contains the information for the compression
 * @param blockSize        The size in bytes of the data that every write operation writes to the file. (must be power of 2)
 */
void compressFile(const std::string& filename, const std::string& output_filename, ASCIIHuffman *huffman, uint16_t blockSize);

#endif
