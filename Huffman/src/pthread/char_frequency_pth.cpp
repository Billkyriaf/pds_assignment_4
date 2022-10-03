#include <iostream>
#include <pthread.h>

#include "char_frequency_pth.h"
#include "file_utils_pth.h"

//#define DEBUG_MODE

typedef struct freq_args {
    int t_id;
    uint64_t *freq_arr;
    const char *filename;

    uint64_t start_byte;  // inclusive
    uint64_t end_byte;  // exclusive

} FreqArgs;


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

    /*// Determine the scan_size of the file
    if (file_len > SCAN_SIZE)
        scan_size = SCAN_SIZE;
    else
        scan_size = file_len;*/

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
void *frequencyRunnable(void *args) {
    // Type cast the arguments
    auto *arguments = (FreqArgs *) args;

    // Each thread creates it's own file handler
    FILE *file = openBinaryFile(arguments->filename, "rb");

    // Count the frequencies of the part of the file assigned
    charFrequency(file, arguments->freq_arr, arguments->start_byte, arguments->end_byte);

    fclose(file);  // close the file

    pthread_exit(nullptr);
}


/**
 * Divides the frequency calculation work between threads and then creates the threads to actually calculate the
 * frequencies
 *
 * @param filename The input file name (to be compressed)
 * @param huffman  The huffman struct
 */
void calculateFrequency(const char *filename, ASCIIHuffman *huffman) {
    FILE *file = openBinaryFile(filename, "rb");

    // Get the file length
    fseek(file, 0, SEEK_END);  // Jump to the end of the file
    uint64_t file_len = ftell(file);  // Get the current byte offset in the file

    rewind(file);  // Jump back to the beginning of the file

    // Initialize the thread attributes
    pthread_attr_t pthread_custom_attr;
    pthread_attr_init(&pthread_custom_attr);

    // The frequencies array is initialised.
    for (auto & frequency : huffman->frequencies) {
        for (int j = 0; j < 256; ++j) {
            frequency[j] = 0;
        }
    }

    FreqArgs thread_args[N_THREADS]; // The struct array containing the arguments for every thread
    pthread_t threads[N_THREADS];  // Array of all the threads

    uint64_t b_per_thr;  // The number of bytes every thread will process
    uint64_t last_b_per_thr;  // The number of bytes the first thread will process

    // Determine the bytes each thread will process
    if (file_len % N_THREADS == 0) {
        b_per_thr = last_b_per_thr = file_len / N_THREADS;

    } else {
        b_per_thr = file_len / N_THREADS;
        last_b_per_thr = b_per_thr + file_len % N_THREADS;
    }

    for (int i = 0; i < N_THREADS; ++i) {
        // Create the thread's arguments
        thread_args[i].t_id = i;
        thread_args[i].freq_arr = huffman->frequencies[i];
        thread_args[i].filename = filename;

        if (i == N_THREADS - 1) {
            thread_args[i].start_byte = i * b_per_thr;
            thread_args[i].end_byte = i * b_per_thr + last_b_per_thr;

        } else {
            thread_args[i].start_byte = i * b_per_thr;
            thread_args[i].end_byte = i * b_per_thr + b_per_thr;
        }

        // Create the thread
        pthread_create(threads + i, &pthread_custom_attr, frequencyRunnable, (void *) &thread_args[i]);

#ifdef DEBUG_MODE
        cout << "Thread: " << i << " calculating frequencies..." << endl;
#else
        if(thread_args[i].t_id == 0) {
            std::cout << N_THREADS << " threads calculating frequencies..." << std::endl;
        }
#endif

    }

    // Join all the threads
    for (unsigned long thread: threads) {
        pthread_join(thread, nullptr);
    }

#ifdef DEBUG_MODE
    cout << "Accumulating..." << endl;
#endif

    // Accumulate all the frequencies
    for (auto &frequency: huffman->frequencies) {
        for (int j = 0; j < 256; ++j) {
            huffman->charFreq[j] += frequency[j];
        }
    }

    // Delete the attributes
    pthread_attr_destroy(&pthread_custom_attr);

    // Close the file
    fclose(file);
}