#include <iostream>

#include "../timer.h"
#include "../structs.h"
#include "../huffman.h"
#include "../file_utils.h"
#include "char_frequency.h"
#include "compress.h"
#include "decompress.h"

//#define DEBUG_MODE

using namespace std;


int main(int argc, char **argv) {
    // The huffman struct
    ASCIIHuffman huffman;

    // Timer used to measure execution time
    Timer timer;
    Timer overall_timer;
    Timer all;

    if (argc != 2) {
        cout << "Wrong number of arguments. Expected 1 got " << argc - 1 << endl;
        cout << "To run this executable run sequential.out path/to/data/file" << endl;
        return -1;
    }

    string input_file_name = argv[1];
    string output_file_name = input_file_name + ".huff";
    string decompressed_file_name = input_file_name + ".dec";

    // Initialize the frequency array and the symbols array
    for (int i = 0; i < 256; ++i) {
        huffman.charFreq[i] = 0;
        huffman.symbols[i].symbol_length = 0;
        huffman.symbols[i].symbol = 0;
    }

    startTimer(&overall_timer);
    startTimer(&all);

    cout << "Calculating frequencies..." << endl;

    startTimer(&timer);

    charFrequency(input_file_name, &huffman);

    stopTimer(&timer);

    cout << "Frequency elapsed time: ";
    displayElapsed(&timer);

    #ifdef DEBUG_MODE
        cout << "Characters frequency: " << endl;

        // Print the frequency array
        for (int i = 0; i < 255; ++i) {
            cout << huffman.charFreq[i] << ", ";
        }

        cout <<  huffman.charFreq[255] << endl;
    #endif

    cout << "Calculating symbols..." << endl;

    startTimer(&timer);

    createHuffmanTree(&huffman);

    stopTimer(&timer);

    cout << "Symbols elapsed time: ";
    displayElapsed(&timer);

    #ifdef DEBUG_MODE
        cout << "\n\nHuffman symbols: \n" << endl;

        // Print the symbol array
        for (Symbol &symbol : huffman.symbols) {
            cout << "len: " << unsigned(symbol.symbol_length) << ", sym: " << hex << symbol.symbol << dec << endl;
        }
    #endif

    cout << "Compressing file..." << endl;

    startTimer(&timer);

    compressFile(input_file_name, output_file_name, &huffman, 8192 * 4);

    stopTimer(&timer);

    cout << "Compression elapsed time: ";
    displayElapsed(&timer);

    stopTimer(&overall_timer);

    cout << "Overall compression elapsed time: ";
    displayElapsed(&overall_timer);

    cout << "Decompressing file..." << endl;

    startTimer(&timer);

    decompressFile(output_file_name, decompressed_file_name);

    stopTimer(&timer);

    cout << "\nOverall decompression elapsed time: ";
    displayElapsed(&timer);

    stopTimer(&all);
    cout << "Overall elapsed time: ";
    displayElapsed(&all);

    // Check if the decompressed file is the same as the original
    verifyFiles(input_file_name, decompressed_file_name);

    return 0;
}
