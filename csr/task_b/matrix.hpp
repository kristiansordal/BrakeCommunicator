#pragma once
#include <omp.h>
#include <vector>
using namespace std;
using i64 = int64_t;
using ulli = unsigned long long int;

class Matrix {
  public:
    i64 n;
    i64 nrows;
    i64 ncols;
    i64 nnz;
    vector<i64> row_ptr;
    vector<i64> col_ptr;
    vector<i64> row_sizes;
    vector<double> v_old;
    vector<double> v_new;
    vector<double> vals;

    Matrix() = default;
    ~Matrix() = default;

    void update();
    void init_row_ptr();
    void init_row_size();
    void init_v_old();
    void init_v_new();
};
