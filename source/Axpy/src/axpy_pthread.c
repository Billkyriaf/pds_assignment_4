#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "axpy_utils.h"

#define DEBUG 1  /// If value is 0 no prints are used
#define MAX_THREADS 1024  /// The maximum number of threads created

extern Exec_time execTime;  // Execution time measure

/**
 * Arguments for the runnable function
 */
typedef struct {
    int *x;    /// x vector
    int *y;    /// y vector
    int a;     /// the scalar
    int size;  /// the size of the vectors

    int start_index;  /// The starting index of the x and y vector
    int end_index;    /// The ending index of the x and y vector
} Arguments;

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
 * Runnable function that calculates part of the total problem
 * @param args  Pointer to arguments structure
 * @return
 */
void *axpyRunnable(void *args) {

    // Type cast the arguments
    Arguments *arguments = (Arguments *) args;

    int end;  // The index of the last element assigned to this thread

    // Check if the index is out of bounds
    if (arguments->end_index > arguments->size) {
        // If it is the ending index will be the final element of the vectors
        end = arguments->size;
    } else {
        end = arguments->end_index;
    }

    // For every element assigned to this thread...
    for (int i = arguments->start_index; i < end; ++i) {
        //...calculate the axpy
        arguments->y[i] = arguments->a * arguments->x[i] + arguments->y[i];
    }

    return NULL;
}

/**
 * Finds the value a * x + y for every item in the two vectors and assigns the value to the y vector
 * @param a      The scalar value
 * @param x      The x vector
 * @param y      The y vector
 * @param size   The size of the vectors
 */
void calculateAxpy(int a, int *x, int *y, int size, int n_threads) {

    int grain_size;

    // Find the number of elements each thread will calculate
    // if the threads don't divide the size exactly...
    if (size % n_threads != 0)
        //...add 1 to make sure all elements are calculated. The excess elements will be handled by the last thread
        // int the runnable function.
        grain_size = size / n_threads + 1;
    else
        //...else simple div the size with the number of threads
        grain_size = size / n_threads;

    Arguments *args = (Arguments *) malloc(n_threads * sizeof(Arguments));  // Array of structs for thread arguments
    pthread_t *tid = (pthread_t *) malloc(n_threads * sizeof(pthread_t));   // Array of thread ids

    // For every thread...
    for (int i = 0; i < n_threads; ++i) {
        //... initialize the arguments
        args[i].x = x;
        args[i].y = y;
        args[i].a = a;

        args[i].size = size;
        args[i].start_index = i * grain_size;
        args[i].end_index = (i + 1) * grain_size ;

        // Create the thread
        pthread_create(&tid[i], NULL, axpyRunnable, &args[i]);
    }

    // Join the threads and wait for them to finish
    for (int i = 0; i < n_threads; ++i) {
        pthread_join(tid[i], NULL);
    }

    free(args);
    free(tid);
}

int main(int argc, char **argv) {
    // If the arguments are not correct...
    if (argc != 3) {
        // ...print error message...
        printUsage();

        // ...and return error code 1
        return 1;
    }

    int N = atoi(argv[1]);  /// The size of the vectors
    int n_threads = atoi(argv[2]);  /// The number of threads requested
    int *x;  /// The x vector of size N
    int *y;  /// The y vector of size N
    int a;   /// The scalar value

    execTime.measureStart();

    // Initialize the vectors
    initializeVectors(N, &x, &y, &a);

    execTime.measureEnd();

    printf("\nPreparation time %.4f ms\n", execTime.elapsed_time);

    if (n_threads > MAX_THREADS) {
        n_threads = MAX_THREADS;
        printf("Warning requested more threads that allowed!!\n"
               "You can change the limit by changing the #define value\n");
    }

    if (DEBUG) {
        printf("Vector x:\n");
        printVector(N, x);

        printf("Vector y:\n");
        printVector(N, y);

        printf("\nScalar %d, size: %d\n", a, N);

        printf("\nNumber of threads: %d\n", n_threads);
    }

    execTime.measureStart();

    calculateAxpy(a, x, y, N, n_threads);

    execTime.measureEnd();

    if (DEBUG) {
        printf("Axpy vector:\n");
        printVector(N, y);
    }

    printf("\nExecution time %.4f ms\n", execTime.elapsed_time);

    free(x);
    free(y);

    return 0;
}



