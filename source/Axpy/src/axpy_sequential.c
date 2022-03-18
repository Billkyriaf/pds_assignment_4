#include <stdio.h>
#include <stdlib.h>

#include "axpy_utils.h"

#define DEBUG 1  /// If value is 0 no prints are used

extern Exec_time execTime;  // Execution time measure


/**
 * Prints the error message and the help
 */
void printUsage() {
    printf("Wrong number of arguments!!\n");
    printf("Usage: ....\n");  // FIXME correct the usage

    // TODO add welcome message
    // TODO add help message
}


/**
 * Finds the value a * x + y for every item in the two vectors and assigns the value to the y vector
 * @param a      The scalar value
 * @param x      The x vector
 * @param y      The y vector
 * @param size   The size of the vectors
 */
void calculateAxpy(int a, int *x, int *y, int size) {

    // for every vector element...
    for (int i = 0; i < size; ++i) {

        // ... calculate the axpy value
        y[i] = a * x[i] + y[i];
    }
}


int main(int argc, char **argv) {

    // If the arguments are not correct...
    if (argc != 2){
        // ...print error message...
        printUsage();

        // ...and return error code 1
        return 1;
    }

    int N = atoi(argv[1]);  /// The size of the vectors
    int *x;  /// The x vector of size N
    int *y;  /// The y vector of size N
    int a;   /// The scalar value

    execTime.measureStart();

    // Initialize the vectors
    initializeVectors(N, &x, &y, &a);

    execTime.measureEnd();

    printf("\nPreparation time %.4f ms\n", execTime.elapsed_time);

    if (DEBUG){
        printf("Vector x:\n");
        printVector(N, x);

        printf("Vector y:\n");
        printVector(N, y);

        printf("\nScalar %d\n", a);
    }

    execTime.measureStart();

    calculateAxpy(a, x, y, N);

    execTime.measureEnd();

    if (DEBUG){
        printf("Axpy vector:\n");
        printVector(N, y);
    }

    printf("\nExecution time %.4f ms\n", execTime.elapsed_time);

    free(x);
    free(y);

    return 0;
}
