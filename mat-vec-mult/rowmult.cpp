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

    double start;
    double end;
    double *vector = new double[n];
    double *res = new double[rows];
    double *matrix = new double[matrix_size];
    double *gathered_res = new double[n];

    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            vector[i] = i + 1;
        }
        start = time.elapsed();
    }

    mpi::broadcast(world, vector, n, 0);

    init_matrix_segment(matrix, matrix_size, n, rank);
    mat_mult(matrix, vector, res, rows, n);

    mpi::gather(world, res, rows, gathered_res, 0);

    delete[] matrix, delete[] vector, delete[] res;

    if (rank == 0) {
        end = time.elapsed();

        if (n < 64) {

            for (int i = 0; i < n; i++) {
                std::cout << gathered_res[i] << " ";
            }
            std::cout << std::endl;
        }

        std::cout << "Time taken: " << end - start << std::endl;
    }
    delete[] gathered_res;
    return 0;
}
