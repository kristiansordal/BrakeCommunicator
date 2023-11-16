#include "matrix.hpp"
#include <iostream>
#include <numeric>
#include <omp.h>

void Matrix::update(mpi::communicator &world, int rank, int &ops) {

#pragma omp parallel for schedule(static)
    for (int row = 0; row < n; row++) {
        double sum = 0.0;

        for (int i = col_ptr[row] - col_ptr[0]; i < col_ptr[row + 1] - col_ptr[0]; i++) {
            sum += vals[i] * v_old[row_ptr[i]];
            ops++;
        }

        v_new[row] = sum;
    }
    mpi::all_gather(world, v_new.data(), v_new.size(), v_old.data());
}

void Matrix::init_col_ptr() {
    vector<i64> c;
    int count = 1;

    c.push_back(0);
#pragma omp parallel for schedule(static)
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

void Matrix::init_v(int np) {
    for (int i = 0; i < n; i++) {
        v_old.push_back((double)i / n);
        // v_old.push_back(10);
    }
    v_new.assign(n / np, 0);
}
