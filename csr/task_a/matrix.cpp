#include "matrix.hpp"
#include <iostream>
#include <numeric>
#include <random>

void Matrix::update(mpi::communicator &world, mpi::timer &time, int rank, double &tcomp_kernel, double &tcomm) {
    i64 start = row_ptr[0];
    double t1 = time.elapsed();
#pragma omp parallel for schedule(dynamic, 1024)
    for (int row = 0; row < n; row++) {
        double sum = 0.0;

        for (int i = row_ptr[row] - start; i < row_ptr[row + 1] - start; i++) {
            sum += vals[i] * v_old[col_ptr[i]];
        }

        v_new[row] = sum;
    }
    tcomp_kernel += time.elapsed() - t1;

    vector<vector<double>> send_buff;
    send_buff.assign(world.size(), vector<double>());
    send_buff[rank] = v_new;

    t1 = time.elapsed();
    for (int i = 0; i < world.size(); i++) {
        if (i == rank) {
            for (int j = 0; j < world.size(); j++) {
                if (j != rank) {
                    world.send(j, i, send_buff[i]);
                }
            }
        } else {
            world.recv(i, i, send_buff[i]);
        }
    }
    tcomm += time.elapsed() - t1;

    v_old.clear();
    v_old.reserve(nrows);

    for (auto &i : send_buff) {
        v_old.insert(v_old.end(), i.begin(), i.end());
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

void Matrix::init_v_old(int np) {
    // Seed the random number generator
    // random_device rd;
    // mt19937 gen(rd());
    // uniform_real_distribution<double> dis(-0.5, 0.5);
    for (int i = 0; i < nrows; i++) {
        v_old.push_back(1);
    }
}

void Matrix::init_v_new() { v_new.assign(n, 0); }
