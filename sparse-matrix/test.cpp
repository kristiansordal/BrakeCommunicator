#include <boost/mpi.hpp>
#include <vector>
namespace mpi = boost::mpi;
int main() {
    mpi::environment env;
    mpi::communicator world;
    int rank = world.rank();

    std::vector<int> vec = {0, 0, 0, 0, 0, 0};

    // std::vector<int> send;
    // send.assign(vec.size(), 0);
    // std::vector<int> sep_ids;
    // if (rank == 0) {
    //     sep_ids = {1, 3, 5};
    //     vec[1] = 1;
    //     vec[3] = 2;
    //     vec[5] = 5;
    // }
    // mpi::all_gather(world, send[rank], vec);

    if (rank == 0) {
        for (int i = 0; i < (int)vec.size(); i++) {
            std::cout << vec[i] << " ";
        }
        std::cout << std::endl;
    }
}
