#include <iostream>
void init_matrix(double *matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i * n + j] = i + j + 1;
        }
    }
}

void mat_mult(double *matrix, double *vector, double *res, int n) {
    for (int i = 0; i < n; i++) {
        double sum = 0;
        for (int j = 0; j < n; j++) {
            sum += matrix[i * n + j] * vector[j];
        }

        res[i] = sum;
    }
}

int main() {

    int scale = 2;
    int n = 1 << scale;

    double *vector = new double[n];
    double *res = new double[n];
    double *matrix = new double[n * n];

    for (int i = 0; i < n; i++) {
        vector[i] = i + 1;
    }

    init_matrix(matrix, n);
    mat_mult(matrix, vector, res, n);

    delete[] vector, delete[] matrix;

    for (int i = 0; i < n; i++) {
        std::cout << res[i] << " ";
    }
    delete[] res;

    return 0;
}
