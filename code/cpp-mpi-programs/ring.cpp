#include <boost/mpi.hpp>
#include <iostream>
#include <mpi.h>
#include <string>

namespace mpi = boost::mpi;

void ring() {
    mpi::environment env;
    mpi::communicator world;

    int myrank = world.rank();
    int np = world.size();
    int x;
    mpi::status status;
    mpi::request req;

    std::cout << "I am " << myrank << " of " << np << " processes" << std::endl;

    if (myrank == 0) {
        x = 11;
        world.send((myrank - 1 + np) % np, 0, x);
        world.recv((myrank + 1) % np, 0, x);
        printf("Processor %d got %d from %d \n", myrank, myrank, (myrank - 1 + np) % np);
    } else {
        world.recv((myrank + 1) % np, 0, x);
        printf("Processor %d got %d from %d \n", myrank, x, (myrank + 1) % np);
        world.send((myrank - 1 + np) % np, 0, x);
    }
}
