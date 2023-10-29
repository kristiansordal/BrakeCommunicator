#include "ellpack.cpp"
#include "ellpack.h"

int main() {
    int n = 1 << 9;
    int timesteps = 100;
    double start, end;

    ELLpack<double> ellpack(n);
    ellpack.initialize();
    ellpack.initialize_stiffness_matrix();
    ellpack.initialize_vectors();
    ellpack.determine_separators();

    if (ellpack.rank == 0) {
        start = ellpack.time.elapsed();
    }
    for (int i = 0; i < 1; i++) {
        ellpack.update();
    }

    if (ellpack.rank == 0) {
        end = ellpack.time.elapsed();
        std::cout << "Time elapsed: " << end - start << std::endl;
    }

    return 0;
}
