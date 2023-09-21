#include <boost/mpi.hpp>
#include <boost/mpi/cartesian_communicator.hpp>
#include <stdio.h>
#include <stdlib.h>

namespace mpi = boost::mpi;

namespace gridmult {

void init_cell(double *cell, std::vector<int> coords, int cell_size, int cell_width, int rank, int n) {
    int cells = n * n / cell_size;

    for (int i = 0; i < cell_width; i++) {
        for (int j = 0; j < cell_width; j++) {
            cell[i * cell_width + j] = (i * n + (coords[0] * cells * n)) + (j + (coords[1] * cell_width)) + 1;
        }
    }

    std::cout << "I am: " << rank << std::endl;
    for (int i = 1; i <= cell_size; i++) {
        std::cout << cell[i - 1] << " ";
        if (i % cell_width == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

void mat_mult(double *cell, double *vector, double *res, int cell_width, int n) {
    // appearently i need to do some communication here
    for (int i = 0; i < n; i++) {

        for (int j = 0; j < cell_width; j++) {
            res[i] += cell[j * n + i] * vector[j];
        }
    }
}

void gridmult() {
    mpi::environment env;
    mpi::communicator world;
    mpi::timer time;

    int rank = world.rank();
    int np = world.size();
    int scale = 3;
    int n = 1 << scale;
    int cell_size = (n * n) / np;
    int cell_width = floor(sqrt(cell_size));

    // Create a cartesian rankink system for processes in order to be able to
    // properly populate the matrix
    int dims[2] = {n / cell_width, n / cell_width};
    int periods[2] = {false, false};
    mpi::cartesian_topology topology(dims, periods);
    mpi::cartesian_communicator cart_comm(world, topology);
    auto coords = cart_comm.coordinates(rank);

    auto squares = {1, 4, 16, 64, 128};

    if (std::find(squares.begin(), squares.end(), np) == squares.end()) {
        std::cout << "Number of processes must be quadratic" << std::endl;
        return;
    }

    double start;
    double end;
    double *vector = new double[n];
    double *cell = new double[cell_size];

    init_cell(cell, coords, cell_size, cell_width, rank, n);

    // if (rank == 0) {
    //     for (int i = 0; i < n; i++) {
    //         vector[i] = i + 1;
    //     }
    //     start = time.elapsed();
    // }
}

} // namespace gridmult
