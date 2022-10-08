#include <cilk/cilk.h>

#include "char_frequency_cilk.h"
#include "../file_utils.h"

//#define DEBUG_MODE


typedef struct job_args {
    uint64_t *freq_arr = nullptr;
    std::string filename;

    uint64_t start_byte = 0;  // inclusive
    uint64_t end_byte = 0;  // exclusive

} JobArgs;


/**
 * Counts the character frequency of every ascii char of the file being compressed.
 *
 * @param file           The file to count the frequencies
 * @param frequency_arr  The frequency array of the characters
 * @param start_byte     The byte (inclusive, measuring from 0) from where to start counting in the file
 * @param end_byte       The byte (exclusive, measuring from 0) to where to stop counting in the file
 */
void charFrequency(FILE *file, uint64_t *frequency_arr, uint64_t start_byte, uint64_t end_byte) {

    uint8_t c;  // The character read from the file

    fseek(file, (long)start_byte, SEEK_SET);  // Jump to the beginning of the section to count

    uint64_t file_len = end_byte - start_byte;  // The number of bytes to read from the file

    unsigned long int scan_size = file_len;

    // Read from the file byte by byte
    for (uint64_t i = 0; i < scan_size; ++i) {
        fread(&c, sizeof(c), 1, file);

        // For every half char read update the frequency
        frequency_arr[c]++;
    }
}


/**
 * Runnable function that calculates the characters frequency of a part of a file. The file is simultaneously handled
 * by multiple threads each having it's own handler. Every thread reads and counts a different part of the file, from
 * start_byte to end byte
 *
 * @param args FreqArgs struct containing the arguments of the function
 * @return
 */
void frequencyRunnable(JobArgs *arguments) {
    // Each thread creates it's own file handler
    FILE *file = openBinaryFile(arguments->filename, "rb");

    // Count the frequencies of the part of the file assigned
    charFrequency(file, arguments->freq_arr, arguments->start_byte, arguments->end_byte);

    fclose(file);  // close the file
}


/**
 * Divides the frequency calculation work between threads and then creates the threads to actually calculate the
 * frequencies
 *
 * @param filename The input file name (to be compressed)
 * @param huffman  The huffman struct
 */
void calculateFrequency(const std::string& filename, ASCIIHuffman *huffman) {
    FILE *file = openBinaryFile(filename, "rb");

    // Get the file length
    fseek(file, 0, SEEK_END);  // Jump to the end of the file
    uint64_t file_len = ftell(file);  // Get the current byte offset in the file

    rewind(file);  // Jump back to the beginning of the file

    // The frequencies array is initialised.
    for (auto & frequency : huffman->frequencies) {
        for (int j = 0; j < 256; ++j) {
            frequency[j] = 0;
        }
    }

    JobArgs job_args[CILK_JOBS]; // The struct array containing the arguments for every thread

    uint64_t bytes_per_job;  // The number of bytes every thread will process
    uint64_t last_bytes_per_job;  // The number of bytes the first thread will process

    // Determine the bytes each thread will process
    if (file_len % CILK_JOBS == 0) {
        bytes_per_job = last_bytes_per_job = file_len / CILK_JOBS;

    } else {
        bytes_per_job = file_len / CILK_JOBS;
        last_bytes_per_job = bytes_per_job + file_len % CILK_JOBS;
    }

    for (int i = 0; i < CILK_JOBS; ++i) {
        // Create the thread's arguments
        job_args[i].freq_arr = huffman->frequencies[i];
        job_args[i].filename = filename;

        if (i == CILK_JOBS - 1) {
            job_args[i].start_byte = i * bytes_per_job;
            job_args[i].end_byte = i * bytes_per_job + last_bytes_per_job;

        } else {
            job_args[i].start_byte = i * bytes_per_job;
            job_args[i].end_byte = i * bytes_per_job + bytes_per_job;
        }

        // Cilk spawn a new thread
        cilk_spawn frequencyRunnable(&job_args[i]);

        #ifdef DEBUG_MODE
                cout << "Thread: " << i << " calculating frequencies..." << endl;
        #else
                if(i == 0) {
                    std::cout << CILK_JOBS << " cilk jobs created and counting frequencies..." << std::endl;
                }
        #endif

    }

    // Sync all the workers
    cilk_sync;

    #ifdef DEBUG_MODE
        cout << "Accumulating..." << endl;
    #endif

    // Accumulate all the frequencies
    for (auto &frequency: huffman->frequencies) {
        for (int j = 0; j < 256; ++j) {
            huffman->charFreq[j] += frequency[j];
        }
    }

    // Close the file
    fclose(file);
}