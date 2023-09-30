#include <boost/mpi.hpp>
#include <stdio.h>
#include <stdlib.h>

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

void mat_mult(double *matrix, double *vector, double *res, int rows, int n) {

    for (int i = 0; i < rows; i++) {
        double sum = 0;

        for (int j = 0; j < n; j++) {
            sum += matrix[i * n + j] * vector[j];
        }

        res[i] = sum;
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
    int rows = n / np;
    int matrix_size = (n * n) / np;

    // timing variables
    int comm_reps = 100;
    double time_sum = 0;
    double bcast_avg;
    double gather_avg;
    double start_bcast;
    double end_bcast;
    double start_matmult;
    double end_matmult;
    double start_gather;
    double end_gather;

    double *vector = new double[n];
    double *res = new double[rows];
    double *matrix = new double[matrix_size];
    double *gathered_res = new double[n];

    init_matrix_segment(matrix, matrix_size, n, rank);

    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            vector[i] = i + 1;
        }
    }

    world.barrier();
    for (int i = 0; i < comm_reps; i++) {
        start_bcast = time.elapsed();
        mpi::broadcast(world, vector, n, 0);
        end_bcast = time.elapsed();
        time_sum += end_bcast - start_bcast;
    }

    bcast_avg = time_sum / comm_reps;
    time_sum = 0;
    world.barrier();

    start_matmult = time.elapsed();
    mat_mult(matrix, vector, res, rows, n);
    end_matmult = time.elapsed();

    world.barrier();

    for (int i = 0; i < comm_reps; i++) {
        start_gather = time.elapsed();
        mpi::gather(world, res, rows, gathered_res, 0);
        end_gather = time.elapsed();
        time_sum += end_gather - start_gather;
    }

    gather_avg = time_sum / comm_reps;

    delete[] matrix, delete[] vector, delete[] res;

    if (rank == 0) {
        std::cout << "Mat mult:  " << end_matmult - start_matmult << std::endl;
        std::cout << "Broadcast: " << bcast_avg << std::endl;
        std::cout << "Gather:    " << gather_avg << std::endl;
    }
    delete[] gathered_res;
    return 0;
}
