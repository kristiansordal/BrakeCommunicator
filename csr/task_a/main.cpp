#include "fast_matrix_market/app/triplet.hpp"
#include "matrix.hpp"
#include <bits/stdc++.h>
#include <boost/mpi.hpp>
#include <fast_matrix_market/fast_matrix_market.hpp>
#include <metis.h>
#include <optional>

using namespace std;
namespace mpi = boost::mpi;
namespace ffm = fast_matrix_market;

// TODO: Implement hybrid version (OpenMP + MPI version) of Compressed Sparse Row sparse
// TODO: Matrix-vector multiplication using  1d or 1e strategy with file  reading.
// TODO: Implement file reading.
// TODO: Use metis for load balancing (PartKwayGraph)
// TODO: Check out https://sparcityeu.github.io/sparsebase/pages/getting_started/ for file reading

void read_file(string path, Matrix &matrix) {
    ifstream file;
    file.open(path);

    if (!file.is_open()) {
        cerr << "Error: Could not open file." << endl;
        return;
    }

    auto get_file_name = [](const std::string &path) {
        size_t found = path.find_last_of("/");
        return (found != std::string::npos) ? path.substr(found + 1) : path;
    };

    std::string file_name = get_file_name(path);

    ffm::read_matrix_market_triplet(file, matrix.nrows, matrix.ncols, matrix.row_ptr, matrix.col_ptr, matrix.vals);
    matrix.nnz = matrix.vals.size();
    matrix.n = matrix.nrows;

    cout << "Successfully read file " << file_name << endl;
    file.close();
}

void split_nnz(Matrix &matrix, mpi::communicator world, int np, int rank) {
    int x;
    int rem;

    if (rank == 0) {
        x = matrix.nnz / np;
        rem = matrix.nnz % np;
    }

    mpi::broadcast(world, x, 0);
    mpi::broadcast(world, rem, 0);
    matrix.nnz = x;

    for (int i = 0; i < rem; i++) {
        if (i == rank) {
            matrix.nnz++;
        }
    }
    cout << "rank: " << rank << " " << matrix.nnz << endl;
}

void split_vals(Matrix &matrix, mpi::communicator world, int np, int rank) {
    std::vector<std::vector<double>> vals;

    int vals_per_rank = matrix.nnz / np;
    int rem = matrix.nnz % np;
}

int main() {
    Matrix matrix;

    mpi::environment env;
    mpi::communicator world;
    mpi::timer time;

    int np = world.size();
    int rank = world.rank();

    // process mtx file on rank 0
    if (rank == 0) {
        read_file("matrices/Arrow.mtx", matrix);
        matrix.init_row_ptr();
    }

    mpi::broadcast(world, matrix.n, 0);
    matrix.row_ptr.resize(matrix.n / np);

    mpi::scatter(world, matrix.row_ptr.data(), matrix.row_ptr.data(), matrix.n / np, 0);

    // send the pointer to the end of the vector to the last rank
    if (rank == 0) {
        world.isend(np - 1, 0, matrix.row_ptr[matrix.n]);
    }

    if (rank == np - 1) {
        int x;
        world.irecv(0, 0, x);
        matrix.row_ptr.push_back(x);
    }

    for (int i = np; i > 0; i++) {
        if (i == rank) {
            int x;
            world.recv(i, 0, x);
            matrix.row_ptr.push_back(x);
        } else if (i == rank - 1) {
            world.send(i - 1, 0, matrix.row_ptr[0]);
        }
    }

    // for (int i = 1; i < np; i++) {
    //     int x;
    //     world.isend(0, 0, matrix.row_ptr[matrix.n]);
    //     world.irecv(i, 0, x);
    //     if (rank == i) {
    //         matrix.row_ptr.push_back(x);
    //     }
    // }
    // for (auto &i : matrix.row_ptr) {
    //     cout << i << " ";
    // }
    // cout << endl;

    if (rank == 0) {

        for (auto &i : matrix.col_ptr) {
            cout << i << " ";
        }
        cout << endl;
    }

    std::vector<std::vector<double>> send_buffer;
    send_buffer.assign(np, std::vector<double>());

    for (int i = 0; i < np; i++) {
        if (i == rank) {
        }
    }

    return 0;
}
