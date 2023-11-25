#include "matrix.hpp"
#include <iostream>
#include <numeric>
#include <random>

void Matrix::update() {
    i64 start = row_ptr[0];
#pragma omp parallel for schedule(static)
    for (int row = 0; row < n; row++) {
        double sum = 0.0;

        for (int i = row_ptr[row] - start; i < row_ptr[row + 1] - start; i++) {
            sum += vals[i] * v_old[col_ptr[i]];
        }

        v_old[row] = sum;
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

void Matrix::init_row_size() {
    for (int i = 0; i < n; i++) {
        row_sizes.push_back(row_ptr[i + 1] - row_ptr[i]);
    }
}

void Matrix::init_v_old() {
    for (int i = 0; i < nrows; i++) {
        v_old.push_back(1);
    }
}

void Matrix::init_v_new() { v_new.assign(n, 0); }
