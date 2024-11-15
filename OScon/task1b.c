#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define TILE_SIZE 32

typedef struct {
    int **A;
    int **B;
    int **C;
    long int M;
    long int N;
    long int K;
    int start_row;
    int end_row;
} ThreadData;

void Initialise_Array(int ***X, long int rows, long int cols) {
    *X = (int **)malloc(rows * sizeof(int *));
    if (*X == NULL) {
        printf("Memory allocation failed in Initialise_Array()!!\n");
        exit(1);
    }

    for (long int i = 0; i < rows; i++) {
        (*X)[i] = (int *)malloc(cols * sizeof(int));
        if ((*X)[i] == NULL) {
            printf("Memory allocation failed in Initialise_Array()!!\n");
            exit(1);
        }
    }

    for (long int i = 0; i < rows; i++) {
        for (long int j = 0; j < cols; j++) {
            (*X)[i][j] = i + j;
        }
    }
}

void Free(int **X, long int rows) {
    for (long int i = 0; i < rows; i++) {
        free(X[i]);
    }
    free(X);
}

// GEMM with tiling/blocking
DWORD WINAPI Gemm_Tiling(LPVOID arg) {
    ThreadData *data = (ThreadData *)arg;
    int **A = data->A;
    int **B = data->B;
    int **C = data->C;
    long int M = data->M;
    long int N = data->N;
    long int K = data->K;
    int start_row = data->start_row;
    int end_row = data->end_row;

    for (long int ii = start_row; ii < end_row; ii += TILE_SIZE) {
        for (long int jj = 0; jj < N; jj += TILE_SIZE) {
            for (long int kk = 0; kk < K; kk += TILE_SIZE) {

                // Process each tile
                for (long int i = ii; i < ii + TILE_SIZE && i < M; i++) {
                    for (long int j = jj; j < jj + TILE_SIZE && j < N; j++) {
                        int sum = 0;
                        for (long int k = kk; k < kk + TILE_SIZE && k < K; k++) {
                            sum += A[i][k] * B[k][j];
                        }
                        C[i][j] += sum;
                    }
                }
            }
        }
    }
    return 0;
}

// Wrapper function for parallel matrix multiplication
void Parallel_Matrix_Multiplication(int **A, int **B, int **C, long int M, long int N, long int K, int num_threads) {
    HANDLE threads[num_threads];
    ThreadData thread_data[num_threads];

    int rows_per_thread = M / num_threads;
    
    // Initialize C matrix to 0
    for (long int i = 0; i < M; i++) {
        for (long int j = 0; j < N; j++) {
            C[i][j] = 0;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].A = A;
        thread_data[i].B = B;
        thread_data[i].C = C;
        thread_data[i].M = M;
        thread_data[i].N = N;
        thread_data[i].K = K;
        thread_data[i].start_row = i * rows_per_thread;
        thread_data[i].end_row = (i == num_threads - 1) ? M : (i + 1) * rows_per_thread;

        threads[i] = CreateThread(NULL, 0, Gemm_Tiling, &thread_data[i], 0, NULL);
        if (threads[i] == NULL) {
            printf("Error creating thread %d\n", i);
            exit(1);
        }
    }

    // Wait for all threads to complete
    WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);

    // Close thread handles
    for (int i = 0; i < num_threads; i++) {
        CloseHandle(threads[i]);
    }
}

// Function to get high-resolution time difference in seconds
double get_time_in_seconds() {
    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    // This would be where you call your main function to time
    QueryPerformanceCounter(&end);

    return (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
}

int main() {
    long int M = 500, N = 500, K = 500;
    char ans;
    int num_threads;

    printf("Do you want to enter M, K, N manually? (y/n): ");
    scanf(" %c", &ans);

    if (ans == 'y' || ans == 'Y') {
        printf("Enter the values of M, K, N: ");
        scanf("%ld %ld %ld", &M, &K, &N);
    }

    printf("Enter the number of threads to use: ");
    scanf("%d", &num_threads);

    int **A, **B, **C;

    Initialise_Array(&A, M, K);
    Initialise_Array(&B, K, N);
    Initialise_Array(&C, M, N);

    // Start timing
    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    Parallel_Matrix_Multiplication(A, B, C, M, N, K, num_threads);

    // End timing
    QueryPerformanceCounter(&end);
    double time_taken = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;

    printf("\n------------------------------------------------------\n\n");
    printf("Exec time of Parallel Matrix Mult: %lf seconds\n", time_taken);
    printf("\n------------------------------------------------------\n\n");

    Free(A, M);
    Free(B, K);
    Free(C, M);

    return 0;
}
