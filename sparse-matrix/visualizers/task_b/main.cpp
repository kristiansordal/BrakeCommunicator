#include "ellpack.cpp"
#include "ellpack.h"
#include "visualizer.cpp"

int main() {
    int n = 1 << 2;

    ELLpack<double> ellpack(n);
    ellpack.initialize();
    ellpack.initialize_stiffness_matrix();
    ellpack.initialize_vectors();
    ellpack.determine_separators();
    ellpack.reorder_separators();
    ellpack.update();

    // if (ellpack.rank == 0) {
    //     visualize(&ellpack);

    // } else {
    //     while (true) {
    //         ellpack.update();
    //     }
    // }

    return 0;
}
