#include <stdio.h>
#include <stdlib.h>

#include <boost/mpi.hpp>

namespace mpi = boost::mpi;

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
            // res[i] += matrix[j * n + i] * vector[j];
            res[i] += matrix[i * n + j] * vector[j];
        }
    }
}

int main() {
    mpi::environment env;
    mpi::communicator world;
    mpi::timer time;

    int rank = world.rank();
    int np = world.size();
    int scale = 2;
    int n = 1 << scale;
    int matrix_size = (n * n) / np;

    // size of a column
    int cols = n / np;

    // timing variables
    double start_total;
    double end_total;

    double *vector = new double[n];

    // part of the vector to multiply with for each rank
    double *vector_slice = new double[cols];

    // result vector for each rank
    double *res = new double[cols];
    double *matrix = new double[matrix_size];
    double *gathered_res = new double[n];

    init_matrix_segment(matrix, matrix_size, n, rank);

    world.barrier();

    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            vector[i] = i + 1;
        }
        start_total = time.elapsed();
    }

    mpi::scatter(world, vector, vector_slice, cols, 0);
    mat_mult(matrix, vector_slice, res, cols, n);

    mpi::reduce(world, res, n, gathered_res, std::plus<double>(), 0);

    world.barrier();

    if (rank == 0) {
        end_total = time.elapsed();
        std::cout << "Time:      " << end_total - start_total << std::endl;
        for (int i = 0; i < n; i++) {
            std::cout << gathered_res[i] << std::endl;
        }
    }

    delete[] matrix;
    delete[] vector;
    delete[] vector_slice;
    delete[] res;
    delete[] gathered_res;

    return 0;
}
