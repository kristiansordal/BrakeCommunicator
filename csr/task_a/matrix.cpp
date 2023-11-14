#include "matrix.hpp"
#include <numeric>
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

void Matrix::init_row_ptr() {
    vector<i64> r;

    int count = 1;
    r.push_back(0);
    for (int i = 1; i < row_ptr.size(); i++) {
        if (row_ptr[i] == row_ptr[i - 1]) {
            count++;
        } else {
            r.push_back(count++);
        }
    }

    r.push_back(count);
    row_ptr = r;
}

void Matrix::init_col_ptr() {
    vector<i64> c;
    int count = 1;

    for (int i = 1; i < col_ptr.size(); i++) {
        if (col_ptr[i] == col_ptr[i - 1]) {
            count++;
        } else {
            c.push_back(count++);
        }
    }
    c.push_back(count);
    col_ptr = c;
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
