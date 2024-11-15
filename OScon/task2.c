#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

#define BUFFER_SIZE 5
#define M 20
#define N 20
#define MAX_MATRICES 20

typedef struct {
    int data[M][N];
} Matrix;

typedef struct {
    Matrix matrix1;
    Matrix matrix2;
} MatrixPair;

// Buffer structure
typedef struct {
    MatrixPair buffer[BUFFER_SIZE];
    int front;
    int rear;
    int count;
    HANDLE mutex;
    HANDLE not_full;
    HANDLE not_empty;
} Buffer;

Buffer B;
int matrices_produced = 0;
int matrices_consumed = 0;
HANDLE count_mutex;

// Initialize buffer
void Init_Buffer() {
    B.front = B.rear = B.count = 0;
    B.mutex = CreateMutex(NULL, FALSE, NULL);
    B.not_full = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, NULL);
    B.not_empty = CreateSemaphore(NULL, 0, BUFFER_SIZE, NULL);
    count_mutex = CreateMutex(NULL, FALSE, NULL);
}

// Fill matrices
void Fill_Matrix(Matrix *X) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            X->data[i][j] = rand() % 10;
        }
    }
}

// Function to multiply two matrices with tiling
void Gemm_Tiling(Matrix *A, Matrix *B, Matrix *C) {
    int tileSize = 8;  // Set a tile size appropriate to cache

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            C->data[i][j] = 0;
        }
    }

    for (int ii = 0; ii < M; ii += tileSize) {
        for (int jj = 0; jj < N; jj += tileSize) {
            for (int kk = 0; kk < M; kk += tileSize) {
                // Process each tile
                for (int i = ii; i < ii + tileSize && i < M; i++) {
                    for (int j = jj; j < jj + tileSize && j < N; j++) {
                        int sum = 0;
                        for (int k = kk; k < kk + tileSize && k < M; k++) {
                            sum += A->data[i][k] * B->data[k][j];
                        }
                        C->data[i][j] += sum;
                    }
                }
            }
        }
    }
}

// Producer function
DWORD WINAPI Producer(LPVOID arg) {
    while (1) {
        WaitForSingleObject(count_mutex, INFINITE);
        if (matrices_produced >= MAX_MATRICES) {
            ReleaseMutex(count_mutex);
            break;
        }
        matrices_produced++;
        ReleaseMutex(count_mutex);

        MatrixPair matrices;
        Fill_Matrix(&matrices.matrix1);
        Fill_Matrix(&matrices.matrix2);

        WaitForSingleObject(B.not_full, INFINITE);
        WaitForSingleObject(B.mutex, INFINITE);

        B.buffer[B.rear] = matrices;
        B.rear = (B.rear + 1) % BUFFER_SIZE;
        B.count++;

        printf("Producer inserted matrices into buffer[%d]\n", B.rear);

        ReleaseMutex(B.mutex);
        ReleaseSemaphore(B.not_empty, 1, NULL);
    }
    return 0;
}

// Consumer function
DWORD WINAPI Consumer(LPVOID arg) {
    while (1) {
        WaitForSingleObject(count_mutex, INFINITE);
        if (matrices_consumed >= MAX_MATRICES) {
            ReleaseMutex(count_mutex);
            break;
        }
        matrices_consumed++;
        ReleaseMutex(count_mutex);

        WaitForSingleObject(B.not_empty, INFINITE); // lock
        WaitForSingleObject(B.mutex, INFINITE);

        MatrixPair matrices = B.buffer[B.front];
        B.front = (B.front + 1) % BUFFER_SIZE;
        B.count--;

        Matrix result; // function call for multiplication.
        Gemm_Tiling(&matrices.matrix1, &matrices.matrix2, &result);

        printf("Consumer processed matrix from buffer[%d]\n", B.front);

        ReleaseMutex(B.mutex);
        ReleaseSemaphore(B.not_full, 1, NULL);
    }
    return 0;
}

int main() {
    Init_Buffer();

    int num_producers, num_consumers;
    printf("Enter number of producers: ");
    scanf("%d", &num_producers);
    printf("Enter number of consumers: ");
    scanf("%d", &num_consumers);

    HANDLE *producers = (HANDLE *)malloc(num_producers * sizeof(HANDLE));
    HANDLE *consumers = (HANDLE *)malloc(num_consumers * sizeof(HANDLE));

    // Start timing
    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    for (int i = 0; i < num_producers; i++) {
        producers[i] = CreateThread(NULL, 0, Producer, NULL, 0, NULL);
    }
    for (int i = 0; i < num_consumers; i++) {
        consumers[i] = CreateThread(NULL, 0, Consumer, NULL, 0, NULL);
    }

    // Wait for all threads to complete
    WaitForMultipleObjects(num_producers, producers, TRUE, INFINITE);
    WaitForMultipleObjects(num_consumers, consumers, TRUE, INFINITE);

    QueryPerformanceCounter(&end);

    // Calculate and print execution time
    double elapsed_time = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
    printf("\nDone!!\n");
    printf("Total execution time: %.3f seconds\n", elapsed_time);

    // Clean up
    for (int i = 0; i < num_producers; i++) {
        CloseHandle(producers[i]);
    }
    for (int i = 0; i < num_consumers; i++) {
        CloseHandle(consumers[i]);
    }
    free(producers);
    free(consumers);

    CloseHandle(B.mutex);
    CloseHandle(B.not_full);
    CloseHandle(B.not_empty);
    CloseHandle(count_mutex);

    return 0;
}
