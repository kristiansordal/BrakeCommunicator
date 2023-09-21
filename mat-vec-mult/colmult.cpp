#include <boost/mpi.hpp>
#include <stdio.h>
#include <stdlib.h>

namespace mpi = boost::mpi;

namespace colmult {
// Initialize a matrix segment
void init_matrix_segment(double *matrix, int matrix_size, int n, int rank) {
    // Value of j is set in order to achieve index(i,j) = i + j
    for (int i = 0, j = rank * matrix_size / n; i < matrix_size; i++) {
        if (i > 0 && i % n == 0) {
            j++;
        }
        matrix[i] = (i % n) + j + 1;
    }
}

void mat_mult(double *matrix, double *vector, double *res, int cols, int n) {
    for (int i = 0; i < n; i++) {

        for (int j = 0; j < cols; j++) {
            res[i] += matrix[j * n + i] * vector[j];
        }
    }
}

void colmult() {
    mpi::environment env;
    mpi::communicator world;
    mpi::timer time;

    int rank = world.rank();
    int np = world.size();
    int scale = 3;
    int n = 1 << scale;
    int cols = n / np;
    int matrix_size = (n * n) / np;

    double start;
    double end;
    double *vector = new double[n];
    double *vector_slice = new double[cols];
    double *res = new double[cols];
    double *matrix = new double[matrix_size];
    double *gathered_res = new double[n];

    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            vector[i] = i + 1;
        }
        start = time.elapsed();
    }

    // Scatter part of the vector to each process
    mpi::scatter(world, vector, vector_slice, cols, 0);

    init_matrix_segment(matrix, matrix_size, n, rank);
    mat_mult(matrix, vector_slice, res, cols, n);

    // Reduce the result into the gathered result vector
    mpi::reduce(world, res, n, gathered_res, std::plus<double>(), 0);

    delete[] matrix;
    delete[] vector;
    delete[] vector_slice;
    delete[] res;

    if (rank == 0) {
        end = time.elapsed();
        std::cout << "Time taken: " << end - start << std::endl;

        for (int i = 0; i < n; i++) {
            std::cout << gathered_res[i] << " ";
        }

        std::cout << std::endl;
    }
    delete[] gathered_res;
}

} // namespace colmult
