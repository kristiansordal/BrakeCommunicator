#include <boost/mpi.hpp>
#include <boost/mpi/cartesian_communicator.hpp>
#include <stdio.h>
#include <stdlib.h>

namespace mpi = boost::mpi;

struct MPI_Env {
  private:
    int periods[2] = {false, false};

  public:
    int n;
    mpi::environment env;
    mpi::communicator world;
    mpi::timer time;
    int np = world.size();
    int rank = world.rank();
    int cell_size;
    int cell_width;
    int dims[2];
    mpi::cartesian_topology *topology;
    mpi::cartesian_communicator *cart_comm;
    std::vector<int> coords;

    MPI_Env(int scale) {
        n = 1 << scale;
        cell_size = (n * n) / np;
        cell_width = floor(sqrt(cell_size));
        dims[0] = {n / cell_width};
        dims[1] = {n / cell_width};
        topology = new mpi::cartesian_topology(dims, periods);
        cart_comm = new mpi::cartesian_communicator(world, *topology);
        coords = cart_comm->coordinates(rank);
    }

    ~MPI_Env() {
        delete topology;
        delete cart_comm;
    }
};

namespace gridmult {

void init_cell(double *cell, MPI_Env *env) {
    int n = env->n;
    int cells = n * n / env->cell_size;
    int cw = env->cell_width;
    int x = env->coords[0], y = env->coords[1];

    for (int i = 0; i < cw; i++) {
        for (int j = 0; j < cw; j++) {
            cell[i * cw + j] = (i * n + (n * x * cw)) + (j + (y * cw)) + 1;
        }
    }

    std::cout << "I am: " << env->rank << std::endl;
    for (int i = 1; i <= env->cell_size; i++) {
        std::cout << cell[i - 1] << " ";
        if (i % cw == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

// void grid_mult(MPI_Env *env, )

// Scatter the vector to the ranks with 0 as their x-coordinate
void scatter_vector(MPI_Env *env, mpi::communicator *row_comm, double *vector_slice) {
    double *vector = new double[env->n];

    for (int i = 0; i < env->n; i++) {
        vector[i] = i + 1;
    }

    if (env->cart_comm->coordinates(env->rank)[0] == env->cart_comm->coordinates(0)[0]) {
        mpi::scatter(*row_comm, vector, vector_slice, env->cell_width, 0);
    }
}

void propagate_downwards(MPI_Env *env, double *vector_slice) {
    mpi::communicator col_comm = env->world.split(env->coords[1]);
    mpi::broadcast(col_comm, vector_slice, env->cell_width, 0);
}

void gridmult() {

    MPI_Env env(3);
    auto squares = {1, 4, 16, 64};

    if (std::find(squares.begin(), squares.end(), env.np) == squares.end()) {
        std::cout << "Number of processes must be quadratic" << std::endl;
        return;
    }

    double start;
    double end;
    double *cell = new double[env.cell_size];
    double *vector_slice = new double[env.cell_width];

    init_cell(cell, &env);

    mpi::communicator row_comm = env.world.split(env.coords[0]);

    scatter_vector(&env, &row_comm, vector_slice);
    if (env.rank == 0) {
        start = env.time.elapsed();
    }

    propagate_downwards(&env, vector_slice);

    std::cout << "Rank: " << env.rank << " has in the vector slice: ";
    for (int i = 0; i < env.cell_width; i++) {
        std::cout << " " << vector_slice[i];
    }
    std::cout << std::endl;
}

} // namespace gridmult
