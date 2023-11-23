#include "matrix.hpp"
#include <iostream>
#include <numeric>
#include <omp.h>

void Matrix::update(mpi::communicator &world, mpi::timer &time, int rank, i64 &ops, double &tcomp, double &tcomm) {
    double t1 = time.elapsed();
#pragma omp parallel for schedule(dynamic, 1024)
    i64 start = row_ptr[0];
    for (int row = 0; row < n; row++) {
        double sum = 0.0;

        for (int i = row_ptr[row] - start; i < row_ptr[row + 1] - start; i++) {
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

    vector<mpi::request> send_reqs;
    vector<mpi::request> recv_reqs;
    t1 = time.elapsed();
    for (int i = 0; i < world.size(); i++) {
        if (i == rank) {
            for (int j = 0; j < world.size(); j++) {
                if (j != rank) {
                    // world.send(j, i, send_buff[i]);
                    send_reqs.push_back(world.isend(j, i, send_buff[i]));
                }
            }
        } else {
            // world.recv(i, i, send_buff[i]);
            recv_reqs.push_back(world.irecv(i, i, send_buff[i]));
        }
    }

    mpi::wait_all(recv_reqs.begin(), recv_reqs.end());

    tcomm += time.elapsed() - t1;
    v_old.clear();
    v_old.reserve(nrows);

    for (const auto &i : send_buff) {
        v_old.insert(v_old.end(), i.begin(), i.end());
    }
}

void Matrix::init_row_ptr() {
    vector<i64> r;
    int count = 1;

    r.push_back(0);
#pragma omp parallel for schedule(dynamic, 1024)
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
#pragma omp parallel for schedule(dynamic, 1024)
    for (int i = 0; i < n; i++) {
        row_sizes.push_back(row_ptr[i + 1] - row_ptr[i]);
    }
}

void Matrix::init_v_old(int np) {
#pragma omp parallel for schedule(dynamic, 1024)
    for (int i = 0; i < nrows; i++) {
        v_old.push_back(0.99);
    }
}

void Matrix::init_v_new() { v_new.assign(n, 0); }
