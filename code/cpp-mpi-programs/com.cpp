#include <boost/mpi.hpp>
#include <iostream>
#include <mpi.h>
#include <string>

namespace mpi = boost::mpi;
void communicate() {

    int myrank;
    int np;
    int x;
    int i;

    mpi::status status;
    mpi::environment env;
    mpi::communicator world;

    MPI_Comm_rank(world, &myrank);
    MPI_Comm_size(world, &np);

    std::cout << "I am " << myrank << " out of " << np << " processes." << std::endl;

    if (myrank == 0) {
        sleep(2);
        std::cout << "I have slept" << std::endl;

        int y = 0;
        for (i = 1; i < np; i++) {
            std::cout << "Sending to: " << i << std::endl;
            world.send(i, 0, i);
            world.recv(mpi::any_source, 0, x);
            y += x;
        }
        std::cout << y << std::endl;
    } else {
        world.recv(0, 0, x);
        x *= 2;
        world.send(0, 0, x);
    }
}
