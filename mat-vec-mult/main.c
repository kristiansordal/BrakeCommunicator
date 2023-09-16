#include "rowmult.c"
#include "rowmult_seq.c"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    rowmult(argc, argv);
    // rowmult_seq();
    return 0;
}
