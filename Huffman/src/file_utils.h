#ifndef FILE_UTILS_PTH_H
#define FILE_UTILS_PTH_H

#include <iostream>

/**
 * Opens a file in binary format for reading
 *
 * @param filename The file name
 * @param mode     The mode to open the file
 * @return  The FILE pointer created
 */
FILE *openBinaryFile(const std::string& filename, const char *mode);


/**
 * Calculates the sha256 hash of the input and output files and compares the results
 * @param input_file
 * @param output_file
 */
void verifyFiles(const std::string& input_file, const std::string& output_file);

#endif
