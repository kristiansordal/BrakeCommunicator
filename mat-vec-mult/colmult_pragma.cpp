#include <boost/mpi.hpp>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

namespace mpi = boost::mpi;

// Initialize a matrix segment
void init_matrix_segment(double *matrix, int matrix_size, int n, int rank) {
// Value of j is set in order to achieve index(i,j) = i + j
#pragma omp parallel for
    for (int i = 0, j = rank * matrix_size / n; i < matrix_size; i++) {
        if (i > 0 && i % n == 0) {
            j++;
        }
        matrix[i] = (i % n) + j + 1;
    }
}

void mat_mult(double *matrix, double *vector, double *res, int cols, int n) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
#pragma omp parallel for
        for (int j = 0; j < cols; j++) {
            res[i] += matrix[j * n + i] * vector[j];
        }
    }
}

int main() {
    mpi::environment env;
    mpi::communicator world;
    mpi::timer time;

    int rank = world.rank();
    int np = world.size();
    int scale = 15;
    int n = 1 << scale;
    int cols = n / np;
    int matrix_size = (n * n) / np;

    // timing variables
    double start_total;
    double end_total;

    double *vector = new double[n];
    double *vector_slice = new double[cols];
    double *res = new double[cols];
    double *matrix = new double[matrix_size];
    double *gathered_res = new double[n];

    init_matrix_segment(matrix, matrix_size, n, rank);

    world.barrier();

    if (rank == 0) {
#pragma omp parallel for
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
    }

    delete[] matrix;
    delete[] vector;
    delete[] vector_slice;
    delete[] res;
    delete[] gathered_res;

    return 0;
}
