#include "matrix.hpp"
#include <iostream>
#include <numeric>
#include <omp.h>

void Matrix::update(mpi::communicator &world, mpi::timer &time, int rank, int &ops, double &tcomp, double &tcomm) {
    double t1 = time.elapsed();
    for (int row = 0; row < n; row++) {
        double sum = 0.0;

        for (int i = row_ptr[row] - row_ptr[0]; i < row_ptr[row + 1] - row_ptr[0]; i++) {
            sum += vals[i] * v_old[col_ptr[i]];
            ops++;
        }

        v_new[row] = sum;
    }
    tcomp += time.elapsed() - t1;

    vector<vector<double>> send_buff;
    send_buff.assign(world.size(), vector<double>());

    for (int i = 0; i < world.size(); i++) {
        if (rank == i) {
            send_buff[i] = v_new;
        }
    }

    t1 = time.elapsed();
    for (int i = 0; i < world.size(); i++) {
        if (i == rank) {
            for (int j = 0; j < world.size(); j++) {
                if (j != rank) {
                    world.isend(j, i, send_buff[i]);
                }
            }
        } else {
            world.irecv(i, i, send_buff[i]);
        }
    }
    tcomm += time.elapsed() - t1;

    v_old.clear();
    for (auto &i : send_buff) {
        for (auto &j : i) {
            v_old.push_back(j);
        }
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
    for (int i = 0; i < nrows; i++) {
        v_old.push_back(1);
    }
}

void Matrix::init_v_new() { v_new.assign(n, 0); }
