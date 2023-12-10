#include "ellpack.cpp"
#include "ellpack.h"

int main() {
    int n = 1 << 9;
    int timesteps = 10;
    double start, end;

    ELLpack<double> ellpack(n);
    ellpack.initialize();
    ellpack.initialize_stiffness_matrix();
    ellpack.initialize_vectors();
    ellpack.determine_separators();

    if (ellpack.rank == 0) {
        start = ellpack.time.elapsed();
    }
    for (int i = 0; i < timesteps; i++) {
        ellpack.update();
    }

    std::vector<double> vv;

    mpi::gather(ellpack.world, ellpack.v_new.data(), ellpack.size_rank(), vv, 0);
    if (ellpack.rank == 0) {
        double ops = (long long)n * 8ll * 100ll;
        double l2 = 0;

        for (auto &v : vv) {
            l2 += v * v;
        }

        l2 = std::sqrt(l2);

        end = ellpack.time.elapsed();
        std::cout << "Time:    " << end - start << std::endl;
        std::cout << "Comp:    " << (end - start) - ellpack.comm_time << std::endl;
        std::cout << "Comm:    " << ellpack.comm_time << std::endl;
        std::cout << "GFLOPS:  " << ops / (end - start) << std::endl;
        std::cout << "L2:      " << l2 << std::endl;
    }

    return 0;
}
