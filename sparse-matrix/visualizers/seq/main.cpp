#include "visualizer.cpp"
#include <SFML/Graphics.hpp>
int main() {

    int n = 1 << 4;

    ELLpack<double> ellpack(n);
    ellpack.initialize();
    ellpack.initialize_stiffness_matrix();
    ellpack.initialize_vectors();

    visualize(&ellpack);

    return 0;
}
