#include "ellpack.cpp"
#include "ellpack.h"

int main() {
    int n = 1 << 10;

    ELLpack<double> ellpack(n);
    ellpack.initialize();
    ellpack.initialize_stiffness_matrix();
    ellpack.initialize_vectors();
    float start = 0;
    float end = 0;

    ellpack.world.barrier();
    if (ellpack.rank == 0) {
        start = ellpack.time.elapsed();
    }
    for (int i = 0; i < 1000; i++) {
        ellpack.update();
    }
    if (ellpack.rank == 0) {
        end = ellpack.time.elapsed();
        std::cout << "Time: " << end - start << std::endl;
    }

    return 0;
}
