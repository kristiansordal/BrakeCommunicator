#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// Initialize a matrix segment
// TODO: Currently produces the same matrix for all of them - need to fix
// so that it produces the previous matrix + 1
void init_matrix_segment(double *matrix, int matSize, int n, int rank) {
    for (int i = 0, j = rank; i < matSize; i++) {
        if (i > 0 && i % n == 0) {
            j++;
        }
        matrix[i] = (i % n) + j;
        printf("%d ", (i % n) + j);
    }
    printf("\n");
}

double *mat_mul(double *matrix, double *vector, double *res, int matSize, int n) {
    int rows = matSize / n;
    // double *res = (double *)malloc(rows * sizeof(double));

    // TODO: Handle error case when matSize == n (or something idk)
    for (int i = 0, j = 0; i < matSize; i++) {
        if (i > 0 && i % n == 0) {
            j++;
            printf("J: %d", j);
        }

        matrix[j * rows + i] *= vector[i % n];
        printf("Id: %d = %f\n", i, matrix[j * rows + i]);
    }

    double sum = 0;

    for (int i = 0, j = 0; i < matSize; i++) {
        if (i > 0 && i % n == 0) {
            res[j] = sum;
            printf("sum: %f", sum);
            printf("%f", res[j]);
            sum = 0;
            j++;
        }

        sum += matrix[j * rows + i];
        printf("%f + %f = %f\n", sum, matrix[j * rows + i], sum + matrix[j * rows + i]);
    }

    for (int i = 0; i < rows; i++) {
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

    const int scale = 2;
    const int n = 1 << scale;

    double *vector = (double *)malloc(n * sizeof(double));

    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            vector[i] = i;
        }
    }

    MPI_Bcast(vector, n, MPI_DOUBLE, 0, world);

    int matSize = (n * n) / np;
    printf("Rank: %d, MAT SIZE: %d\n", rank, matSize);

    // Creates a matrix segment
    double *matrix = (double *)malloc(matSize * sizeof(double));
    init_matrix_segment(matrix, matSize, n, rank);

    // double *res = mat_mul(matrix, vector, matSize, n);
    double *res = (double *)malloc(sizeof(double) * matSize / n);
    mat_mul(matrix, vector, res, matSize, n);

    for (int i = 0; i < matSize / n; i++) {
        printf("%f\n\n", res[i]);
    }
    MPI_Finalize();
    return 0;
}
