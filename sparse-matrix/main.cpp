#include "ellpack.cpp"
#include "ellpack.h"

int main() {
    int n = 1 << 2;

    ELLpack<double> ellpack(n);
    ellpack.initialize();
    ellpack.initialize_stiffness_matrix();
    ellpack.initialize_vectors();

    if (ellpack.rank == 0) {
        ellpack.print_v();
    }
    for (int i = 0; i < 2; i++) {
        ellpack.update();
        if (ellpack.rank == 0) {
            ellpack.print_v();
        }
        ellpack.world.barrier();
    }

    return 0;
}
