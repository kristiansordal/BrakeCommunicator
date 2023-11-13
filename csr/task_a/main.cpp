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
    cout << matrix.nnz << endl;

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

    std::cout << "rank: " << rank << " " << matrix.nnz << std::endl;
}

int main() {
    mpi::environment env;
    mpi::communicator world;
    mpi::timer time;

    int np = world.size();
    int rank = world.rank();

    Matrix matrix;

    // process mtx file on rank 0
    if (rank == 0) {
        read_file("matrices/494_bus.mtx", matrix);
    }

    split_nnz(matrix, world, np, rank);

    // broadcast matrix data to all processes
    // then scatter col_ptr to all processes
    mpi::scatter(world, matrix.col_ptr.data(), matrix.col_ptr.data(), matrix.nnz, 0);

    for (auto &i : matrix.col_ptr) {
        cout << i << " ";
    }
    cout << endl;

    return 0;
}
