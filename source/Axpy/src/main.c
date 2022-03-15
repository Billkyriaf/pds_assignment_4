/**
 * Program that solves the axpy problem for vectors of size N
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#define MAX_NUMBER 10  /// The maximum element inside the vectors
#define DEBUG 1  /// If value is 0 no prints are used


void printUsage();
void initializeVectors(int size, int *x, int *y, int *a);
void printVector(int size, int *vector);
void calculateAxpy(int a, int *x, int *y, int size);


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

    x = (int *)malloc(N * sizeof(int));
    y = (int *)malloc(N * sizeof(int));

    // Initialize the vectors
    initializeVectors(N, x, y, &a);

    if (DEBUG){
        printf("Vector x:\n");
        printVector(N, x);

        printf("Vector y:\n");
        printVector(N, y);
    }

    struct timeval start_time;
    struct timeval stop_time;

    gettimeofday(&start_time, NULL);

    calculateAxpy(a, x, y, N);

    gettimeofday(&stop_time, NULL);


    if (DEBUG){
        printf("Axpy vector:\n");
        printVector(N, y);
    }

    printf(
            "\nExecution time %.4f ms\n",
            (float)(stop_time.tv_sec * 1000000 - start_time.tv_sec * 1000000 + stop_time.tv_usec - start_time.tv_usec) / 1000
    );

    free(x);
    free(y);

    return 0;
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

/**
 * Initializes the x y vectors to random values. The pointers must be allocated first
 * @param size   The size of the vectors
 * @param x      The x vector
 * @param y      The y vector
 * @param a      The a scalar
 */
void initializeVectors(int size, int *x, int *y, int *a) {
    srand(time(NULL));  // seed for the random number generator

    *a = rand() % MAX_NUMBER;  // init a

    // init the two vectors with random numbers
    for (int i = 0; i < size; ++i) {
        x[i] = rand() % MAX_NUMBER;
        y[i] = rand() % MAX_NUMBER;
    }
}


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
 * Prints a vector to the standard output
 * @param size    The size of the vector
 * @param vector  The vector to print
 */
void printVector(int size, int *vector){
    printf("[");

    if (size <= 20) {
        for (int i = 0; i < size - 1; ++i) {
            printf("%d, ", vector[i]);
        }

    } else {
        for (int i = 0; i < 9; ++i) {
            printf("%d, ", vector[i]);
        }

        printf("%d ... ", vector[9]);

        for (int i = size - 10; i < size; ++i) {
            printf("%d, ", vector[i]);
        }
    }

    printf("%d]\n", vector[size - 1]);
}
