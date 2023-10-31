#include "ellpack.cpp"
#include "ellpack.h"

int main() {
    int n = 1 << 14;

    ELLpack<double> ellpack(n);
    ellpack.initialize();
    ellpack.initialize_stiffness_matrix();
    ellpack.initialize_vectors();
    double start, end;

    if (ellpack.rank == 0) {
        start = ellpack.time.elapsed();
    }
    for (int i = 0; i < 100; i++) {
        ellpack.update();
    }

    std::vector<double> v;
    mpi::gather(ellpack.world, ellpack.v_old.data(), ellpack.size_rank(), v, 0);

    double ops = (long long)n * 8ll * 100ll;

    if (ellpack.rank == 0) {
        double l2 = 0;
        for (auto &vv : v) {
            l2 += vv * vv;
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
