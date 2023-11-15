#include "matrix.hpp"
#include <iostream>
#include <numeric>
#include <omp.h>

void Matrix::update(int rank) {
    for (int row = 0; row < n; row++) {
        double sum = 0.0;

        for (int i = col_ptr[row] - col_ptr[0]; i < col_ptr[row + 1] - col_ptr[0]; i++) {
            sum += vals[i] * v_old[row_ptr[i]];
        }

        v_new[rank * n + row] = sum;
    }
    v_old = v_new;
}

void Matrix::init_col_ptr() {
    vector<i64> c;
    int count = 1;

    c.push_back(0);
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
        v_old.push_back(1.0 / n);
    }
    v_new.assign(v_old.size(), 0);
}
