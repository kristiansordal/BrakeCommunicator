#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// Initialize a matrix segment
void init_matrix_segment(double *matrix, int matSize, int n, int rank) {
    for (int i = 0, j = rank * matSize / n; i < matSize; i++) {
        if (i > 0 && i % n == 0) {
            j++;
        }
        matrix[i] = (i % n) + j + 1;
    }
    printf("\n");
}

double *mat_mul(double *matrix, double *vector, /* double *res, */ int matSize, int n) {
    int rows = matSize / n;
    double *res = (double *)malloc(rows * sizeof(double));

    // TODO: Handle error case when matSize == n (or something idk)
    for (int i = 0; i < matSize; i++) {
        matrix[i] *= vector[i % n];
    }

    double sum = 0;

    for (int i = 1, j = 0; i <= matSize; i++) {
        sum += matrix[i - 1];
        if (i % n == 0) {
            res[j] = sum;
            sum = 0;
            j++;
        }
    }

    return res;
}
int main(int argc, char **argv) {
    int rank;
    int np;
    MPI_Comm world = MPI_COMM_WORLD;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(world, &rank);
    MPI_Comm_size(world, &np);

    const int scale = 10;
    const int n = 1 << scale;

    double *vector = (double *)malloc(n * sizeof(double));

    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            vector[i] = i + 1;
        }
    }

    MPI_Bcast(vector, n, MPI_DOUBLE, 0, world);

    int matSize = (n * n) / np;

    // Creates a matrix segment
    double *matrix = (double *)malloc(matSize * sizeof(double));
    init_matrix_segment(matrix, matSize, n, rank);

    double *res = mat_mul(matrix, vector, matSize, n);

    for (int i = 0; i < matSize / n; i++) {
        printf("%2.2f ", res[i]);
    }

    MPI_Finalize();
    return 0;
}
