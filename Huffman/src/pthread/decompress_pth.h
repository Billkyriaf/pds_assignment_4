#ifndef DECOMPRESS_PTH_H
#define DECOMPRESS_PTH_H

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
 * @param decompressed_filename The name of the decompressed file
 */
void decompressFile(const std::string& filename, const std::string& decompressed_filename);

#endif
