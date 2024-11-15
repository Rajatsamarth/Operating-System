//#define _POSIX_C_SOURCE 199309L // Ensures clock_gettime is available on POSIX-compliant systems
#include <stdio.h>
#include <stdlib.h> 
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Function to initialize a 2D array
void Initialise_Array(int ***X, long int rows, long int cols) {
    *X = (int **)malloc(rows * sizeof(int *));
    if (*X == NULL) {
        printf("Memory allocation failed in Initialise_Array()!!\n");
        exit(1);
    }

    for (long int i = 0; i < rows; i++) {
        (*X)[i] = (int *)malloc(cols * sizeof(int)); // pointer point to memory block capable  of holding for row .
        if ((*X)[i] == NULL) {  // if malloc fails *x is NULL then print the error message
            printf("Memory allocation failed in Initialise_Array()!!\n");
            exit(1);// stop the program using error(1)
        }
    }

    // Initialize values
    for (long int i = 0; i < rows; i++) {
        for (long int j = 0; j < cols; j++) {
            (*X)[i][j] = i + j; // each element of the array is set to the sum of row and column
        }
    }
}

// Function to free a 2D array
void Free(int **X, long int rows) {
    for (long int i = 0; i < rows; i++) {
        free(X[i]); //releases the memory that was allocated for individual row.
    }
    free(X); // releases the memory that was allocated to hold these row pointer.
}

// Function to measure execution time of provided function
double Measure_Execution_Time(void (*func)(int **, int **, int **, long int, long int, long int), int **A, int **B, int **C, long int M, long int N, long int K) {
    #ifdef _WIN32
    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);//gets the frequency of the high resolution  timer 
    QueryPerformanceCounter(&start);//records the current time at the start of the function

    // Call the function
    func(A, B, C, M, N, K);
     // end of the function
    QueryPerformanceCounter(&end);
    //substrating the start time from the end time  and dividing the frequency to convert result.
    double time_taken = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
    #else
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Call the function
    func(A, B, C, M, N, K);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    #endif

    return time_taken;
}

// GEMM with inner product
void Gemm_IP(int **A, int **B, int **C, long int M, long int N, long int K) {
    for (long int i = 0; i < M; i++) {
        for (long int j = 0; j < N; j++) {
            C[i][j] = 0;// defore performing the multi each element in the result matrix initialize zero.
            for (long int k = 0; k < K; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// GEMM with outer product
void Gemm_OP(int **A, int **B, int **C, long int M, long int N, long int K) {
    for (long int k = 0; k < K; k++) {
        for (long int i = 0; i < M; i++) {
            for (long int j = 0; j < N; j++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// GEMM with middle loop
void Gemm_ML(int **A, int **B, int **C, long int M, long int N, long int K) {
    for (long int i = 0; i < M; i++) {
        for (long int k = 0; k < K; k++) {
            for (long int j = 0; j < N; j++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// GEMM with tiling/blocking
void Gemm_Tiling(int **A, int **B, int **C, long int M, long int N, long int K) {
    int tileSize = 64;
   //before starting multi we set all elem of the result matrix c to 0.
    for (long int i = 0; i < M; i++) {
        for (long int j = 0; j < N; j++) {
            C[i][j] = 0;
        }
    }

    for (long int ii = 0; ii < M; ii += tileSize) {// ii iterates the row of matrix A and B.
        for (long int jj = 0; jj < N; jj += tileSize) { // jj iterates the col of matrix B and C
            for (long int kk = 0; kk < K; kk += tileSize) {// kk iterates the common dimension bet A and B

                // Process each tile
                for (long int i = ii; i < ii + tileSize && i < M; i++) {
                    for (long int j = jj; j < jj + tileSize && j < N; j++) {
                        int sum = 0;
                        for (long int k = kk; k < kk + tileSize && k < K; k++) {
                            sum += A[i][k] * B[k][j];// we calculate partial sum by multiplying corresp elements form a and b
                        }
                        C[i][j] += sum;
                    }
                }
            }
        }
    }
}

int main() {
    long int M = 500, N = 500, K = 500;
    char ans;

    printf("Do you want to enter M, K, N manually? (y/n): ");
    scanf(" %c", &ans); 

    if (ans == 'y' || ans == 'Y') {
        printf("Enter the values of M, K, N: ");
        scanf("%ld %ld %ld", &M, &K, &N);
    }

    int **A, **B, **C;

    // Initialize A, B, and C matrices
    Initialise_Array(&A, M, K);
    Initialise_Array(&B, K, N);
    Initialise_Array(&C, M, N);

    // Experimenting with different GEMM variants to find the best one
    printf("\n-------------------------------------------\n\n");
    printf("Exec time of Gemm_IP: %lf seconds\n", Measure_Execution_Time(Gemm_IP, A, B, C, M, N, K));
    printf("Exec time of Gemm_OP: %lf seconds\n", Measure_Execution_Time(Gemm_OP, A, B, C, M, N, K));
    printf("Exec time of Gemm_ML: %lf seconds\n", Measure_Execution_Time(Gemm_ML, A, B, C, M, N, K));
    printf("Exec time of Gemm_Tiling: %lf seconds\n", Measure_Execution_Time(Gemm_Tiling, A, B, C, M, N, K));
    printf("\n-------------------------------------------\n\n");

    // Free the memory allocated dynamically
    Free(A, M);
    Free(B, K);
    Free(C, M);

    return 0;
}
