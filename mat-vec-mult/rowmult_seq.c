#include <stdio.h>
#include <stdlib.h>

// Initialize a matrix segment
void init_matrix(double *matrix, int n) {
    // Value of j is set in order to achieve index(i,j) = i + j
    for (int i = 0; i < n; i++) {
        double *row = (double *)malloc(n * sizeof(double));
        for (int j = 0; j < n; j++) {
            row[j] = i + j;
        }
        matrix[i] = *row;
        free(row);
    }
}

void mat_mul_seq(double *matrix, double *vector, double *res, int n) {
    int rows = n;

    for (int i = 0; i < rows; i++) {
        double sum = 0;

        for (int j = 0; j < n; j++) {
            sum += matrix[i * n + j] * vector[j];
        }

        res[i] = sum;
    }
}

void rowmult_seq() {
    double start;
    double end;

    const int scale = 2;
    const int n = 1 << scale;

    double *vector = (double *)malloc(n * sizeof(double));
    double *res = (double *)malloc(n * sizeof(double));

    for (int i = 0; i < n; i++) {
        vector[i] = i + 1;
    }

    int matSize = (n * n);

    // Creates a matrix segment
    double *matrix = (double *)malloc(matSize * sizeof(double));

    init_matrix(matrix, n);

    mat_mul_seq(matrix, vector, res, n);

    free(matrix);
    free(vector);

    // printf("Scwle: %d\nTime taken: %f\n", scale, end - start);
    for (int i = 0; i < 10; i++) {
        printf("%2.2f ", res[i]);
    }

    free(res);
}
