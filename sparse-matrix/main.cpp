#include "ellpack.cpp"
#include "ellpack.h"
// #include <SFML/Graphics.hpp>
// #include <boost/mpi.hpp>
int main() {

    int n = 1 << 2;

    ELLpack<double> ellpack(n);
    ellpack.initialize();
    ellpack.initialize_stiffness_matrix();
    ellpack.initialize_vectors();

    for (int i = 0; i < 10; i++) {
        ellpack.update();
        if (ellpack.rank == 0) {
            ellpack.print_v();
            std::cout << std::endl;
        }
    }
    if (ellpack.rank == 0) {
        ellpack.print_v();
    }

    return 0;
}
