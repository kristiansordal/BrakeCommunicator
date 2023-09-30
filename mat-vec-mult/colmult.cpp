#include <boost/mpi.hpp>
#include <stdio.h>
#include <stdlib.h>

namespace mpi = boost::mpi;

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
    int comm_reps = 100;
    double time_sum = 0;
    double scatter_avg;
    double reduce_avg;
    double start_scatter;
    double end_scatter;
    double start_matmult;
    double end_matmult;
    double start_reduce;
    double end_reduce;
    double *vector = new double[n];
    double *vector_slice = new double[cols];
    double *res = new double[n];
    double *matrix = new double[matrix_size];
    double *gathered_res = new double[n];

    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            vector[i] = i + 1;
        }
    }

    init_matrix_segment(matrix, matrix_size, n, rank);

    world.barrier();

    for (int i = 0; i < comm_reps; i++) {
        start_scatter = time.elapsed();
        mpi::scatter(world, vector, vector_slice, cols, 0);
        end_scatter = time.elapsed();
        time_sum += end_scatter - start_scatter;
    }

    scatter_avg = time_sum / comm_reps;
    time_sum = 0;

    world.barrier();

    start_matmult = time.elapsed();
    mat_mult(matrix, vector_slice, res, cols, n);
    end_matmult = time.elapsed();

    world.barrier();

    for (int i = 0; i < comm_reps; i++) {
        start_reduce = time.elapsed();
        mpi::reduce(world, res, n, gathered_res, std::plus<double>(), 0);
        end_reduce = time.elapsed();
        time_sum += end_reduce - start_reduce;
    }

    reduce_avg = time_sum / comm_reps;

    delete[] matrix, delete[] vector, delete[] vector_slice, delete[] res;

    if (rank == 0) {
        std::cout << "Mat mult:  " << end_matmult - start_matmult << std::endl;
        std::cout << "Broadcast: " << scatter_avg << std::endl;
        std::cout << "Gather:    " << reduce_avg << std::endl;
    }

    delete[] gathered_res;

    return 0;
}
