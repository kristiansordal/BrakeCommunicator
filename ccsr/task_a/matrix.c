#include "matrix.h"

void init_v_old(struct Matrix *M) {
    for (int i = 0; i < M->n; i++) {
        M->v_old[i] = 1.0;
    }
}
