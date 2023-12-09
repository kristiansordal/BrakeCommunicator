#pragma once
#include <mpi.h>

typedef long long ll;
struct Matrix {
    int n;
    int nrows;
    int ncols;
    int nnz;
    int *row_ptr;
    int *col_ptr;
    double *v_old;
    double *v_new;
    double *vals;
};

void init_v_old(struct Matrix *m);
