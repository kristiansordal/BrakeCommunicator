#include <boost/mpi.hpp>
#include <cmath>
#include <iostream>
#include <mpi.h>
#include <string>

namespace mpi = boost::mpi;
using ll = long long;

int main() {
    mpi::environment env;
    mpi::communicator world;
    mpi::status status;
    mpi::request req;

    const int scale = 2;
    const ll n = 1 << scale;

    int rank = world.rank();
    // int np = world.size();
    // int x = 10;

    if (rank == 0) {
        // std::array<double, n> vec;
        std::vector<double> vec;
        for (int i = 0; i < n; i++) {
            vec[i] = i;
        }
        mpi::broadcast(world, *vec.data(), 0);

    } else {
        // std::array<double, n> vec;
        std::vector<double> vec;
        mpi::broadcast(world, *vec.data(), 0);
        for (auto n : vec) {
            std::cout << n;
        }
        std::cout << std::endl;
    }

    return 0;
}
