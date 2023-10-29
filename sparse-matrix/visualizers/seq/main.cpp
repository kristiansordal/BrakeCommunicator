#include "visualizer.cpp"
#include <chrono>
int main() {

    int n = 1 << 12;

    ELLpack<double> ellpack(n);
    ellpack.initialize();
    ellpack.initialize_stiffness_matrix();
    ellpack.initialize_vectors();

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; i++)
        ellpack.update();
    auto end = std::chrono::high_resolution_clock::now();

    double ops = (long long)n * 8ll * 100ll;
    double l2 = 0;

    for (auto &vv : ellpack.v_old)
        l2 += vv * vv;
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    l2 = std::sqrt(l2);
    std::cout << "Time:    " << duration.count() << std::endl;
    std::cout << "GFLOPS:  " << ops << std::endl;
    std::cout << "L2:      " << l2 << std::endl;
    return 0;
}
