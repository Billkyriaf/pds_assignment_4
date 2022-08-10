#include <cstdio>
#include <iostream>

#include "file_utils.h"
#include "huffman_tree.h"

//#define DEBUG_MODE


using namespace std;

int main() {
    // The huffman struct
    ASCIIHuffman huffman;

    // Initialize the frequency array and the symbols array
    for (int i = 0; i < 256; ++i) {
        huffman.charFreq[i] = 0;
        huffman.symbols[i].symbol_length = 0;
        huffman.symbols[i].symbol = 0;
    }

    FILE *file = openBinaryFile("../data/test_5");

    cout << "Calculating frequencies..." << endl;
    charFrequency(file, &huffman);

#ifdef DEBUG_MODE
    cout << "Characters frequency: " << endl;

    // Print the frequency array
    for (int i = 0; i < 255; ++i) {
        cout << huffman.charFreq[i] << ", ";
    }

    cout <<  huffman.charFreq[255] << endl;
#endif

    cout << "Calculating symbols..." << endl;

    createHuffmanTree(&huffman);

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