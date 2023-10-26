#include "ellpack.cpp"
#include "ellpack.h"
#include "visualizer.cpp"

int main() {
    int n = 1 << 6;

    ELLpack<double> ellpack(n);
    ellpack.initialize();
    ellpack.initialize_stiffness_matrix();
    ellpack.initialize_vectors();
    ellpack.determine_separators();

    if (ellpack.rank == 0) {
        visualize(&ellpack);

    } else {
        while (true) {
            ellpack.update();
        }
    }

    return 0;
}
