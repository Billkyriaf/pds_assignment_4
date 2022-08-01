#include <stdio.h>

#include "file_utils_v3.h"
#include "huffman_tree_v3.h"


int main() {

    // The huffman struct
    HalfASCIIHuffman huffman;

    // Initialize the frequency array and the symbols array
    for (int i = 0; i < 16; ++i) {
        huffman.charFreq[i] = 0;
        huffman.symbols[i].symbol_length = 0;
        huffman.symbols[i].symbol = 0;
    }

    FILE *file = openBinaryFile("../data/test_3");

    charFrequency(file, &huffman);

    printf("Characters frequency: \n");

    // Print the frequency array
    for (int i = 0; i < 15; ++i) {
        printf("%lu, ", huffman.charFreq[i]);
    }

    printf("%lu", huffman.charFreq[15]);

    calculateSymbols(&huffman);

    printf("\n\nHuffman symbols: \n");

    // Print the symbol array
    for (int i = 0; i < 16; ++i) {
        printf("len: %u, sym: %x\n", huffman.symbols[i].symbol_length, huffman.symbols[i].symbol);
    }


    compressFile(file, "../data/test_3", &huffman, 1024);
    decompressFile("../data/test_3.huff");
    fclose(file);
    return 0;
}
