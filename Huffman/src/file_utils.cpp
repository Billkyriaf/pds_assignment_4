#include <cstring>

#include "file_utils.h"

using namespace std;


/**
 * Creates two additional file names one for the huffman compressed file and one for the decompressed one.
 * @param input_file_name   The input file name
 * @param compressed          The compressed file name (.huff)
 * @param decompressed        The decompressed file name (.dec)
 */
void createFileNames(const char *input_file_name, char *compressed, char *decompressed){
    // Create a new file for compression
    int len = (int) strlen(input_file_name); // Get the length of the initial filename
    compressed = (char *) malloc((len + 7) * sizeof(char));  // Allocate enough space for the new file name

    memcpy(compressed, input_file_name, sizeof(char) * len);  // Copy the old name to the new name

    char end_compressed[6] = ".huff";  // The string to be appended to the file name

    memcpy(compressed + len, end_compressed, sizeof(char) * 6);  // Append the ending to the file name


    // Create a new file for decompression
    decompressed = (char *) malloc((len + 6) * sizeof(char));  // Allocate enough space for the new file name

    memcpy(decompressed, input_file_name, sizeof(char) * len);  // Copy the old name to the new name

    char end[5] = ".dec";  // The string to be appended to the file name

    memcpy(decompressed + len, end, sizeof(char) * 5);  // Append the ending to the file name
}


/**
 * Opens a file in binary format for reading
 *
 * @param filename The file name
 * @param mode     The mode to open the file
 * @return  The FILE pointer created
 */
FILE *openBinaryFile(const string& filename, const char *mode) {
    FILE *file = nullptr;
    const char* name = filename.c_str();

    file = fopen(name, mode);

    if (file == nullptr) {
        cout << "File not found..." << endl;
        exit(-1);
    }

    return file;
}

/**
 * Calculates the sha256 hash of the input and output files and compares the results
 * @param input_file
 * @param output_file
 */
void verifyFiles(const string& input_file, const string& output_file){
    cout << "Verifying files..." << endl;

    char input_sha[64];  // The sha256 hash of the input file
    char output_sha[64];  // The sha256 hash of the output file

    string command_1 = "sha256sum " + input_file;  // Create the command to calculate the hash of the input file
    string command_2 = "sha256sum " + output_file;  // Create the command to calculate the hash of the output file

    FILE *input;  // The file pointer to the input pipe
    FILE *output;  // The file pointer to the output pipe

    input = popen(command_1.c_str(), "r");  // Open the input pipe
    output = popen(command_2.c_str(), "r");  // Open the output pipe

    if (input == nullptr || output == nullptr){
        cout << "Error opening the files..." << endl;
        exit(-1);
    }

    fgets(input_sha, 64, input);  // Read the hash of the input file
    fgets(output_sha, 64, output);  // Read the hash of the output file

    // Check if the hashes are the same
    if (strcmp(input_sha, output_sha) == 0){
        cout << "\n  SHA256 TEST PASS" << endl;
        cout << "    " << input_file << "          " << input_sha << endl;
        cout << "    " << output_file << "      "<< output_sha << endl;
    } else {
        cout << "\n  SHA256 TEST FAIL" << endl;
        cout << "    " << input_file << "          " << input_sha << endl;
        cout << "    " << output_file << "      "<< output_sha << endl;
    }

    pclose(input);  // Close the input pipe
    pclose(output);  // Close the output pipe
}