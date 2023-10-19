#include "ellpack.cpp"
#include "ellpack.h"
int main() {

    int n = 1 << 2;

    ELLpack<double> ellpack(n);
    ellpack.initialize();
    ellpack.initialize_stiffness_matrix();
    ellpack.initialize_vectors();
    ellpack.update();

    return 0;
}
