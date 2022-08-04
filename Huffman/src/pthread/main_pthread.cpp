#include <cstdio>
#include <iostream>
#include <pthread.h>

#include "file_utils_pthread.h"

#define DEBUG_MODE
#define N_THREADS 4

using namespace std;

typedef struct freq_args{
    int t_id;
    uint64_t *freq_arr;

    uint64_t start_byte;
    uint64_t end_byte;

}FreqArgs;


/**
 * Runnable function that calculates the characters frequency of a part of a file
 *
 * @param args FreqArgs struct containing the arguments of the function
 * @return
 */
void *frequencyRunnable(void *args){
    // Type cast the arguments
    auto *arguments = (FreqArgs *)args;

    // Each thread creates it's own file handler
    FILE *file = openBinaryFile("../data/test_5");

    // Count the frequencies of the part of the file assigned
    charFrequency(file, arguments->freq_arr, arguments->start_byte, arguments->end_byte);

    fclose(file);  // close the file

    pthread_exit(nullptr);
}


/**
 * Divides the frequency calculation work between threads and then creates the threads to actually calculate the
 * frequencies
 *
 * @param file     The input file (to be compressed)
 * @param huffman  The huffman struct
 */
void calculateFrequency(FILE *file, ASCIIHuffman *huffman){
    // Get the file length
    fseek(file, 0, SEEK_END);  // Jump to the end of the file
    uint64_t file_len = ftell(file);  // Get the current byte offset in the file

    rewind(file);  // Jump back to the beginning of the file

    // Initialize the thread attributes
    pthread_attr_t pthread_custom_attr;
    pthread_attr_init(&pthread_custom_attr);

    // The frequencies array is promoted to a 2D array so every thread can count without locking the array.
    uint64_t frequencies[N_THREADS][256] = {};

    FreqArgs thread_args[N_THREADS]; // The struct array containing the arguments for every thread
    pthread_t threads[N_THREADS];  // Array of all the threads

    uint64_t b_per_thr;  // The number of bytes every thread will process
    uint64_t last_b_per_thr;  // The number of bytes the first thread will process

    // Determine the bytes each thread will process
    if (file_len % N_THREADS == 0){
        b_per_thr = last_b_per_thr = file_len / N_THREADS;

    } else {
        b_per_thr = file_len / N_THREADS;
        last_b_per_thr = b_per_thr + file_len % N_THREADS;
    }

    for (int i = 0; i < N_THREADS; ++i) {
        // Create the thread's arguments
        thread_args[i].t_id = i;
        thread_args[i].freq_arr = frequencies[i];

        if (i == N_THREADS - 1) {
            thread_args[i].start_byte = i * b_per_thr;
            thread_args[i].end_byte = i * b_per_thr + last_b_per_thr + 1;

        } else {
            thread_args[i].start_byte = i * b_per_thr;
            thread_args[i].end_byte = i * b_per_thr + b_per_thr;
        }

        // Create the thread
        pthread_create(threads + i, &pthread_custom_attr, frequencyRunnable, (void *) &thread_args[i]);

#ifdef DEBUG_MODE
        cout << "Thread: " << i << " calculating frequencies..." << endl;
#else
        if(arguments->t_id == 0) {
            cout << N_THREADS << " threads calculating frequencies..." << endl;
        }
#endif

    }

    // Join all the threads
    for (unsigned long thread : threads) {
        pthread_join(thread, nullptr);
    }

    // Accumulate all the frequencies
    for (auto & frequency : frequencies) {
        for (int j = 0; j < 256; ++j) {
            huffman->charFreq[j] += frequency[j];
        }
    }

    // Delete the attributes
    pthread_attr_destroy(&pthread_custom_attr);
}


int main() {
    // The huffman struct
    ASCIIHuffman huffman;

    // Initialize the frequency array and the symbols array
    for (int i = 0; i < 256; ++i) {
        huffman.charFreq[i] = 0;
        huffman.symbols[i].symbol_length = 0;
        huffman.symbols[i].symbol = 0;
    }

    // The main thread also opens the file
    FILE *file = openBinaryFile("../data/test_5");

    // Calculate the frequency of the characters
    calculateFrequency(file, &huffman);

#ifdef DEBUG_MODE
    cout << "Characters frequency: " << endl;

    // Print the frequency array
    for (int i = 0; i < 255; ++i) {
        cout << huffman.charFreq[i] << ", ";
    }

    cout <<  huffman.charFreq[255] << endl;
#endif

    cout << "Calculating symbols..." << endl;

    calculateSymbols(&huffman);

#ifdef DEBUG_MODE
    cout << "\n\nHuffman symbols: \n" << endl;

    // Print the symbol array
    for (Symbol &symbol : huffman.symbols) {
        cout << "len: " << unsigned(symbol.symbol_length) << ", sym: " << hex << symbol.symbol << dec << endl;
    }
#endif

    cout << "Compressing file..." << endl;
    compressFile(file, "../data/test_5", &huffman, 8192 * 4);

    cout << "Decompressing file..." << endl;
    decompressFile("../data/test_5.huff");

    fclose(file);
    return 0;
}
