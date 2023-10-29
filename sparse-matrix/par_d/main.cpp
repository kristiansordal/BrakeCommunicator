#include "ellpack.cpp"
#include "ellpack.h"

int main() {
    int n = 1 << 10;
    int timesteps = 100;
    double start, end;

    std::cout << n << std::endl;
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
        std::cout << "Time:    " << end - start << std::endl;
        std::cout << "Comp:    " << (end - start) - ellpack.comm_time << std::endl;
        std::cout << "Comm:    " << ellpack.comm_time << std::endl;
        std::cout << "GFLOPS:  " << ellpack.comm_time << std::endl;
    }

    double ops = (long long)n * 8ll * 100ll;
    std::cout << ops << std::endl;
    return 0;
}
