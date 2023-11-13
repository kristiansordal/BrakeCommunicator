#include "matrix.hpp"
#include <omp.h>

void Matrix::update() {
    for (int row = 0; row < n; row++) {
        double sum = 0.0;

        for (int i = row_ptr[row]; i < row_ptr[row + 1]; i++) {
            sum += vals[i] * v_old[col_ptr[i]];
        }

        v_new[row] = sum;
    }
}

void Matrix::init_v() {
    for (int i = 0; i < n; i++) {
        v_old[i] = 1.0 / n;
    }
}

void Matrix::init_a_mat() {
    for (int i = 0; i < nnz; i++) {
        vals[i] = 0.3;
    }
}
