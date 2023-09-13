#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// Initialize a matrix segment
void init_matrix_segment(double *matrix, int matSize, int n, int rank) {
    // Value of j is set in order to achieve index(i,j) = i + j
    for (int i = 0, j = rank * matSize / n; i < matSize; i++) {
        if (i > 0 && i % n == 0) {
            j++;
        }
        matrix[i] = (i % n) + j + 1;
    }
}

void mat_mul(double *matrix, double *vector, double *res, int matSize, int n, int rank) {
    int rows = matSize / n;
    int startRow = rank * rows;

    printf("Res Mem adr: %p\n", (void *)&res[0]);
    for (int i = 0; i < rows; i++) {
        double sum = 0;

        for (int j = 0; j < n; j++) {
            printf("%f, %f\n", matrix[i * n + j], vector[j]);
            sum += matrix[i * n + j] * vector[j];
        }

        printf("Inserting %f into res at id: %d\n", sum, startRow + i);
        res[startRow + i] = sum;
    }
}

int main(int argc, char **argv) {
    int rank;
    int np;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    const int scale = 2;
    const int n = 1 << scale;

    double *vector = (double *)malloc(n * sizeof(double));
    double *res = (double *)malloc(n * sizeof(double));

    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            vector[i] = i + 1;
        }
    }
    double start = MPI_Wtime();

    MPI_Bcast(vector, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    int matSize = (n * n) / np;

    // Creates a matrix segment
    double *matrix = (double *)malloc(matSize * sizeof(double));

    init_matrix_segment(matrix, matSize, n, rank);
    mat_mul(matrix, vector, res, matSize, n, rank);

    double end = MPI_Wtime();

    printf("Scale: %d\nTime taken: %f\n", scale, end - start);

    for (int i = 0; i < n; i++) {
        if (res[i] > 0.0000000000) {
            printf("%2.2f ", res[i]);
        }
    }

    MPI_Finalize();
    return 0;
}
