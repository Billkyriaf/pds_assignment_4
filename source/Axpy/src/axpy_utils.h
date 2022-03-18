#ifndef AXPY_AXPY_UTILS_H
#define AXPY_AXPY_UTILS_H

#include <sys/time.h>

#define MAX_NUMBER 10  /// The maximum element inside the vectors

/**
 * Struct used to measure execution time. The struct is initialized in this file and it can be used directly where ever
 * needed
 */
typedef struct{
    struct timeval start_time;  /// The starting time of the benchmark
    struct timeval stop_time;   /// The ending time of the benchmark

    float elapsed_time;  /// The elapsed time of the benchmark

    void (*measureStart)(void);  /// Function to get the starting time
    void (*measureEnd)(void);    /// Function to get the ending time and calculate the elapsed time

}Exec_time;


void initializeVectors(int size, int **x, int **y, int *a);
void printVector(int size, int *vector);
void getStartingTime();
void getEndingTime();

#endif
