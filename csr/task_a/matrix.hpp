#pragma once
#include <vector>
using namespace std;
using i64 = int64_t;

class Matrix {
  public:
    int n;
    int nrows;
    int ncols;
    int nnz;
    vector<i64> row_ptr;
    vector<i64> col_ptr;
    vector<double> v_old;
    vector<double> v_new;
    vector<double> a;

    Matrix() = default;

    void update();
    void init_v();
    void init_a_mat();
};
