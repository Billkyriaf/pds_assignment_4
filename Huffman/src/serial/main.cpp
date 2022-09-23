#include <cstdio>
#include <iostream>

#include "file_utils.h"
#include "huffman_tree.h"
#include "../../include/timer/timer.h"

//#define DEBUG_MODE


using namespace std;

int main() {
    // The huffman struct
    ASCIIHuffman huffman;

    Timer timer;
    Timer overall_timer;

    //    char *input_file_name = argv[1];
    // Create the file names (temp solution)
    const char *input_file_name = "../../data/test_3";
    const char *output_file_name = "../../data/test_3.huff";

    // Initialize the frequency array and the symbols array
    for (int i = 0; i < 256; ++i) {
        huffman.charFreq[i] = 0;
        huffman.symbols[i].symbol_length = 0;
        huffman.symbols[i].symbol = 0;
    }

    startTimer(&overall_timer);

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

    compressFile(input_file_name, &huffman, 8192 * 4);

    stopTimer(&timer);

    cout << "Compression elapsed time: ";
    displayElapsed(&timer);

    cout << "Decompressing file..." << endl;

    startTimer(&timer);

    decompressFile(output_file_name);

    stopTimer(&timer);

    cout << "Decompression elapsed time: ";
    displayElapsed(&timer);

    stopTimer(&overall_timer);
    cout << "Overall elapsed time: ";
    displayElapsed(&overall_timer);

    return 0;
}
