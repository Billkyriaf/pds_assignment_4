#include <cstring>

#include "file_utils_pth.h"

using namespace std;


/**
 * Opens a file in binary format for reading
 *
 * @param filename The file name
 * @param mode     The mode to open the file
 * @return  The FILE pointer created
 */
FILE *openBinaryFile(const char *filename, const char *mode) {
    FILE *file = nullptr;

    file = fopen(filename, mode);

    if (file == nullptr) {
        printf("File not found...\n");
        exit(-1);
    }

    return file;
}

/**
 * Calculates the sha256 hash of the input and output files and compares the results
 * @param input_file
 * @param output_file
 */
void verifyFiles(const char *input_file, const char *output_file){
    cout << "Verifying files..." << endl;

    char command_1[100];  // The command to calculate the hash of the input file
    char command_2[100];  // The command to calculate the hash of the output file

    char input_sha[64];  // The sha256 hash of the input file
    char output_sha[64];  // The sha256 hash of the output file

    sprintf(command_1, "sha256sum %s", input_file);  // Create the command to calculate the hash of the input file
    sprintf(command_2, "sha256sum %s", output_file);  // Create the command to calculate the hash of the output file

    FILE *input;  // The file pointer to the input pipe
    FILE *output;  // The file pointer to the output pipe

    input = popen(command_1, "r");  // Open the input pipe
    output = popen(command_2, "r");  // Open the output pipe

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