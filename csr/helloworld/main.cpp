#include <boost/mpi.hpp>
int main() {

    boost::mpi::environment env;
    boost::mpi::communicator world;

    std::cout << "Hello, world! I am process " << world.rank() << " of " << world.size() << "." << std::endl;

    return 0;
}
