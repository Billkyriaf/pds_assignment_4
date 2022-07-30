#include <stdio.h>

#include "file_utils_v2.h"
#include "huffman_tree_v2.h"


int main() {

    // The huffman struct
    ASCIIHuffman huffman;
    huffman.longest_symbol = 0;

    // Initialize the frequency array and the symbols array
    for (int i = 0; i < 256; ++i) {
        huffman.charFreq[i] = 0;
        huffman.symbols[i].symbol_length = 0;

        for (int j = 0; j < 32; ++j) {
            huffman.symbols[i].symbol[j] = 0;
        }
    }

    FILE *file = openBinaryFile("../data/test_3");

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
    for (int i = 0; i < 255; ++i) {
        printf("%x, ", huffman.symbols[i]);
    }

    printf("%x\n", huffman.symbols[255]);

    compressFile(file, "../data/test_3", &huffman, 1024);
    decompressFile("../data/test_3.huff");
    fclose(file);
    return 0;
}
