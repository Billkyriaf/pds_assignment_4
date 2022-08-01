#include <cstdio>
#include <iostream>

#include "file_utils.h"
#include "huffman_tree.h"


using namespace std;

int main() {
    // The huffman struct
    HalfASCIIHuffman huffman;

    // Initialize the frequency array and the symbols array
    for (int i = 0; i < 256; ++i) {
        huffman.charFreq[i] = 0;
        huffman.symbols[i].symbol_length = 0;
        huffman.symbols[i].symbol = 0;
    }

    FILE *file = openBinaryFile("../data/test_4");

    charFrequency(file, &huffman);

    printf("Characters frequency: \n");

    // Print the frequency array
    for (int i = 0; i < 255; ++i) {
        printf("%lu, ", huffman.charFreq[i]);
    }

    printf("%lu", huffman.charFreq[255]);

    calculateSymbols(&huffman);

    printf("\n\nHuffman symbols: \n");

    // Print the symbol array
    for (Symbol &symbol : huffman.symbols) {
        cout << "len: " << unsigned(symbol.symbol_length) << ", sym: " << hex << symbol.symbol << dec << endl;
    }


    compressFile(file, "../data/test_4", &huffman, 8192);
    decompressFile("../data/test_4.huff");
    fclose(file);
    return 0;
}
