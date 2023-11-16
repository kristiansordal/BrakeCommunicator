#pragma once
#include <boost/mpi.hpp>
#include <vector>
namespace mpi = boost::mpi;
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
    vector<double> vals;

    Matrix() = default;
    ~Matrix() = default;

    void update(mpi::communicator &world, int rank, int &ops);
    void init_col_ptr();
    void init_v(int np);
};
