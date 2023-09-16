#include <array>
#include <boost/mpi.hpp>
#include <iostream>

namespace mpi = boost::mpi;

using matrix = std::vector<std::vector<double>>;
using vector = std::vector<double>;

const int SCALE = 15;
const int N = 1 << SCALE;

void init_matrix_segment(matrix &mat, int rank, int rows) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < N; j++) {
            mat[i][j] = i + j + 1 + (rank * rows);
        }
    }
}

void mat_mult(matrix &mat, vector &vec, vector &res, int rows) {
    for (int i = 0; i < rows; i++) {
        int sum = 0;

        for (int j = 0; j < N; j++) {
            sum += mat[i][j] * vec[j];
        }

        res[i] = sum;
    }
}

void rowmult() {
    mpi::environment env;
    mpi::communicator world;
    mpi::timer time;

    int rank = world.rank();
    int np = world.size();
    int rows = N / np;
    int matrix_size = (N * N) / np;

    double start = 0;
    double end = 0;

    matrix mat(rows, vector(N));
    vector vec(N);
    vector res(rows);

    if (rank == 0) {
        for (int i = 0; i < N; i++) {
            vec[i] = i + 1;
        }
        start = time.elapsed();
    }

    mpi::broadcast(world, vec.data(), N, 0);

    init_matrix_segment(mat, rank, rows);
    mat_mult(mat, vec, res, rows);

    vector gathered_res(N * np);
    mpi::gather(world, res.data(), rows, gathered_res.data(), 0);

    if (rank == 0) {
        end = time.elapsed();
        std::cout << "Time taken: " << end - start << std::endl;
    }
}
