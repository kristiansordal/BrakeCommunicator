#pragma once
#include <boost/mpi.hpp>
#include <omp.h>
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
    vector<int> row_sizes;
    vector<double> v_old;
    vector<double> v_new;
    vector<double> vals;

    Matrix() = default;
    ~Matrix() = default;

    void update(mpi::communicator &world, mpi::timer &time, int rank, i64 &ops, double &tcomp, double &tcomm);
    void init_row_ptr();
    void init_row_size();
    void init_v_old(int np);
    void init_v_new();
};
