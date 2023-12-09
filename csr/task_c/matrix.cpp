#include "matrix.hpp"

// void Matrix::update(mpi::communicator &world, mpi::timer &time, int rank, double &tcomp_kernel, double &tcomm) {
//     i64 start = row_ptr[0];
//     double t1 = time.elapsed();
// #pragma omp parallel for schedule(dynamic, 1024)
//     for (int row = 0; row < n; row++) {
//         double sum = 0.0;

//         for (int i = row_ptr[row] - start; i < row_ptr[row + 1] - start; i++) {
//             sum += vals[i] * v_old[col_ptr[i]];
//         }

//         v_new[row] = sum;
//     }
//     tcomp_kernel += time.elapsed() - t1;

//     // vector<int> recvcounts(world.size(), 0);
//     // int v = v_new.size();
//     // mpi::all_gather(world, &v, 1, recvcounts);
//     // vector<int> displs;
//     // displs.push_back(0);
//     // for (int i = 1; i < recvcounts.size(); i++) {
//     //     displs[i] = displs[i - 1] + recvcounts[i - 1];
//     // }

//     // // Resize v_old to accommodate the gathered data
//     // v_old.clear();
//     // v_old.assign(nrows, 0);

//     // // MPI_Allgatherv for integer data
//     // MPI_Allgatherv(v_new.data(), v, MPI_DOUBLE, v_old.data(), recvcounts.data(), displs.data(), MPI_DOUBLE,
//     world);
//     //
//     vector<vector<double>> send_buff;
//     send_buff.assign(world.size(), vector<double>());
//     send_buff[rank] = v_new;

//     t1 = time.elapsed();
//     for (int i = 0; i < world.size(); i++) {
//         if (i == rank) {
//             for (int j = 0; j < world.size(); j++) {
//                 if (j != rank) {
//                     world.send(j, i, send_buff[i]);
//                 }
//             }
//         } else {
//             world.recv(i, i, send_buff[i]);
//         }
//     }
//     tcomm += time.elapsed() - t1;
//     v_old.clear();
//     v_old.reserve(nrows);
//     for (auto &i : send_buff) {
//         v_old.insert(v_old.end(), i.begin(), i.end());
//     }
//     tcomm += time.elapsed() - t1;
// }

// void Matrix::init_row_ptr() {
//     vector<i64> r;
//     int count = 1;

//     r.push_back(0);
//     for (int i = 1; i < row_ptr.size(); i++) {
//         if (row_ptr[i] == row_ptr[i - 1]) {
//             count++;
//         } else {
//             r.push_back(count++);
//         }
//     }
//     r.push_back(count);
//     row_ptr = r;
// }

// void Matrix::init_v_old(int np) {
//     for (int i = 0; i < nrows; i++) {
//         v_old.push_back(1);
//     }
// }

// void Matrix::init_v_new() { v_new.assign(n, 0); }
