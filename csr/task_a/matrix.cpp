#include "matrix.hpp"
#include <iostream>
#include <numeric>
#include <omp.h>

void Matrix::update(mpi::communicator &world, int rank, int &ops) {

#pragma omp parallel for schedule(static)
    for (int row = 0; row < n; row++) {
        double sum = 0.0;

        for (int i = row_ptr[row] - row_ptr[0]; i < row_ptr[row + 1] - row_ptr[0]; i++) {
            sum += vals[i] * v_old[col_ptr[i]];
            ops++;
        }

        v_new[row] = sum;
    }

    vector<vector<double>> send_buff;

    send_buff.assign(world.size(), vector<double>());

    for (int i = 0; i < world.size(); i++) {
        if (rank == i) {
            send_buff[i] = v_new;
        }
    }
    // if (rank == 1) {
    //     for (auto &i : send_buff) {
    //         for (auto &j : i) {
    //             cout << j << " ";
    //         }
    //         cout << endl;
    //     }
    // }

    for (int i = 0; i < world.size(); i++) {
        if (rank == i) {
            mpi::broadcast(world, send_buff[i].data(), send_buff[i].size(), i);
        }
    }
}

void Matrix::init_row_ptr() {
    vector<i64> c;
    int count = 1;

    c.push_back(0);
#pragma omp parallel for schedule(static)
    for (int i = 1; i < row_ptr.size(); i++) {
        if (row_ptr[i] == row_ptr[i - 1]) {
            count++;
        } else {
            c.push_back(count++);
        }
    }
    c.push_back(count);
    row_ptr = c;
}

void Matrix::init_row_size() {
    for (int i = 0; i < n; i++) {
        row_sizes.push_back(row_ptr[i + 1] - row_ptr[i]);
    }
}

void Matrix::init_v_old(int np) {
    for (int i = 0; i < nrows; i++) {
        v_old.push_back((double)i / nrows);
    }
}

void Matrix::init_v_new() { v_new.assign(n, 0); }
