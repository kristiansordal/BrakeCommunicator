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
}

// Scatter the vector to the ranks with 0 as their x-coordinate
void scatter_vector(MPI_Env *env, double *vector_slice) {
    mpi::communicator row_comm = env->world.split(env->coords[0]);
    double *vector = new double[env->n];

    for (int i = 0; i < env->n; i++) {
        vector[i] = i + 1;
    }

    if (env->cart_comm->coordinates(env->rank)[0] == env->cart_comm->coordinates(0)[0]) {
        mpi::scatter(row_comm, vector, vector_slice, env->cell_width, 0);
    }

    delete[] vector;
}

void propagate_downwards(MPI_Env *env, double *vector_slice) {
    mpi::communicator col_comm = env->world.split(env->coords[1]);
    mpi::broadcast(col_comm, vector_slice, env->cell_width, 0);
}

void mat_mult(MPI_Env *env, double *cell, double *vector_slice, double *res) {
    int cw = env->cell_width;
    for (int i = 0; i < (*env).cell_width; i++) {
        double sum = 0;
        for (int j = 0; j < env->cell_width; j++) {
            sum += cell[i * cw + j] * vector_slice[j];
        }
        res[i] = sum;
    }
}

void reduce_rows(MPI_Env *env, double *res, double *gathered_res) {
    mpi::communicator row_comm = env->world.split(env->coords[0]);
    mpi::reduce(row_comm, res, env->cell_width, gathered_res, std::plus<double>(), 0);
}

void gather_result(MPI_Env *env, double *gathered_res, double *final_res) {
    mpi::communicator col_comm = env->world.split(env->coords[1]);
    mpi::gather(col_comm, gathered_res, env->cell_width, final_res, 0);
}

int main() {

    MPI_Env env(15);
    auto squares = {1, 4, 16, 64};

    if (std::find(squares.begin(), squares.end(), env.np) == squares.end()) {
        std::cout << "Number of processes must be quadratic" << std::endl;
        return 0;
    }

    double start;
    double end;
    double *cell = new double[env.cell_size];
    double *vector_slice = new double[env.cell_width];

    init_cell(cell, &env);

    if (env.rank == 0) {
        start = env.time.elapsed();
    }
    scatter_vector(&env, vector_slice);

    propagate_downwards(&env, vector_slice);

    double *res = new double[env.cell_width];

    mat_mult(&env, cell, vector_slice, res);

    delete[] cell, delete[] vector_slice;

    double *gathered_res = new double[env.cell_width];
    double *final_res = new double[env.n];

    reduce_rows(&env, res, gathered_res);
    gather_result(&env, gathered_res, final_res);

    end = env.time.elapsed();

    delete[] res, delete[] gathered_res;

    if (env.rank == 0) {
        if (env.n < 64) {
            for (int i = 0; i < env.n; i++) {
                std::cout << final_res[i] << " ";
            }
        }
        std::cout << std::endl;

        std::cout << "Time taken: " << end - start << std::endl;
    }

    delete[] final_res;
    return 0;
}
