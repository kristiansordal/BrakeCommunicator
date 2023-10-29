#include "ellpack.cpp"
#include "ellpack.h"

int main() {
    int n = 1 << 10;
    // int n = 1000;
    int timesteps = 100;
    double start, end;

    std::cout << n << std::endl;
    ELLpack<double> ellpack(n);
    // std::cout << ellpack.rank << " here" << std::endl;
    ellpack.initialize();
    // std::cout << ellpack.rank << " initialize" << std::endl;
    ellpack.initialize_stiffness_matrix();
    // std::cout << ellpack.rank << " stiffness" << std::endl;
    ellpack.initialize_vectors();
    // std::cout << ellpack.rank << " vects" << std::endl;
    ellpack.determine_separators();
    // std::cout << ellpack.rank << " seps" << std::endl;

    if (ellpack.rank == 0) {
        start = ellpack.time.elapsed();
    }
    for (int i = 0; i < 1; i++) {
        // std::cout << ellpack.rank << " update" << std::endl;
        ellpack.world.barrier();
        ellpack.update();
    }

    if (ellpack.rank == 0) {
        end = ellpack.time.elapsed();
        std::cout << "Time elapsed: " << end - start << std::endl;
    }

    double ops = (long long)n * 8ll * 100ll;
    std::cout << ops << std::endl;
    return 0;
}
