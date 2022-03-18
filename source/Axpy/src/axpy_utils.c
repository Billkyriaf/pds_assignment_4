#include "axpy_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/// Struct used to measure execution time
Exec_time execTime = {.measureStart = getStartingTime, .measureEnd = getEndingTime};


/**
 * Initializes the x y vectors to random values. x and y are double pointers because arrays are initialized in this
 * function
 *
 * @param size   The size of the vectors
 * @param x      The pointer to the x vector
 * @param y      The pointer to the y vector
 * @param a      The a scalar
 */
void initializeVectors(int size, int **x, int **y, int *a) {
    srand(time(NULL));  // seed for the random number generator

    *a = rand() % MAX_NUMBER;  // init a

    *x = (int *) malloc(size * sizeof(int));
    *y = (int *) malloc(size * sizeof(int));

    // init the two vectors with random numbers
    for (int i = 0; i < size; ++i) {
        (*x)[i] = rand() % MAX_NUMBER;
        (*y)[i] = rand() % MAX_NUMBER;
    }
}


/**
 * Prints a vector to the standard output
 * @param size    The size of the vector
 * @param vector  The vector to print
 */
void printVector(int size, int *vector) {
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


/**
 * Init the start_time struct for time measuring
 */
void getStartingTime() {
    // Init the start time struct
    gettimeofday(&execTime.start_time, NULL);
}

/**
 * Init the stop_time struct for time measuring and calculate the elapsed time.
 */
void getEndingTime() {
    // Init the stop time struct
    gettimeofday(&execTime.stop_time, NULL);

    // Calculate the execution time
    execTime.elapsed_time = (float) (execTime.stop_time.tv_sec * 1000000 - execTime.start_time.tv_sec * 1000000 +
                                     execTime.stop_time.tv_usec - execTime.start_time.tv_usec) / 1000;
}