#include <stdio.h>

#include "file_utils_v3.h"

int main() {

    // The huffman struct
    HalfASCIIHuffman huffman;

    // Initialize the frequency array and the symbols array
    for (int i = 0; i < 16; ++i) {
        huffman.charFreq[i] = 0;
        huffman.symbols[i].symbol_length = 0;
        huffman.symbols[i].symbol = 0;
    }

    FILE *file = openBinaryFile("../data/test_2");

    charFrequency(file, &huffman);

    printf("Characters frequency: \n");

    // Print the frequency array
    for (int i = 0; i < 15; ++i) {
        printf("%lu, ", huffman.charFreq[i]);
    }

    printf("%lu\n\n", huffman.charFreq[15]);

    calculateSymbols(&huffman);

    printf("\n\nHuffman symbols: \n");

    // Print the symbol array
    for (int i = 0; i < 16; ++i) {
        printf("len: %u, sym: %x\n", huffman.symbols[i].symbol_length, huffman.symbols[i].symbol);
    }


    compressFile(file, "../data/test_2", &huffman, 1024);
    decompressFile("../data/test_2.huff");
    fclose(file);
    return 0;
}
