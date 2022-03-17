/**
 * @author Vasilis Kyriafinis
 * @date 17/3/2022
 *
 * @brief
 */
#include <stdio.h>
#include <stdlib.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#include "axpy_utils.h"

#define DEBUG 1  /// If value is 0 no prints are used
#define MAX_WORKERS 1024  /// The maximum number of workers

extern Exec_time execTime;  // Execution time measure


void printUsage(){
    printf("Wrong number of arguments!!\n");
    printf("Usage: ....\n");  // FIXME correct the usage

    // TODO add welcome message
    // TODO add help message
}

/**
 * Calculate the axpy value for every element of the vectors
 * @param a  The scalar
 * @param x  The x vector
 * @param y  The y vector
 * @param size  The size of the vectors
 * @param n_workers  The number of workers
 */
void calculateAxpy(int a, int *x, int *y, int size, int n_workers){
    
    // Set the number of workers
    // cilkrts_set_param("nworkers", n_workers);


    printf("Cilk workers %d\n", __cilkrts_get_nworkers());

    // Find the grain size
    int grain_size = size/n_workers;

    // Set the grain size for every worker
    #pragma cilk grainsize grain_size
    cilk_for (int i = 0; i < size; ++i){

        // Calculate the axpy value
        y[i] = a * x[i] + y[i];
    }

}


int main(int argc, char **argv){
    // If the arguments are not correct...
    if (argc != 3) {
        // ...print error message...
        printUsage();

        // ...and return error code 1
        return 1;
    }

    int N = atoi(argv[1]);  /// The size of the vectors
    int n_workers = atoi(argv[2]);  /// The number of threads requested
    int *x;  /// The x vector of size N
    int *y;  /// The y vector of size N
    int a;   /// The scalar value

    execTime.measureStart();

    // Initialize the vectors
    initializeVectors(N, &x, &y, &a);

    execTime.measureEnd();

    if (n_workers > MAX_WORKERS) {
        n_workers = MAX_WORKERS;
        printf("Warning requested more threads that allowed!!\n"
               "You can change the limit by changing the #define value\n");
    }

    printf("\nPreparation time %.4f ms\n", execTime.elapsed_time);

    if (DEBUG) {
        printf("Vector x:\n");
        printVector(N, x);

        printf("Vector y:\n");
        printVector(N, y);

        printf("\nScalar %d\n", a);

        printf("\nNumber of workers: %d\n", n_workers);
    }

    execTime.measureStart();

    calculateAxpy(a, x, y, N, n_workers);

    execTime.measureEnd();

    if (DEBUG) {
        printf("\nAxpy vector:\n");
        printVector(N, y);
    }

    printf("\nExecution time %.4f ms\n", execTime.elapsed_time);

    free(x);
    free(y);

    return 0;
}

