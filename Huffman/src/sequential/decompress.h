#ifndef DECOMPRESS_H
#define DECOMPRESS_H

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
 * @param decompressed_filename The name of the decompressed file
 */
void decompressFile(const std::string& filename, const std::string& decompressed_filename);

#endif
