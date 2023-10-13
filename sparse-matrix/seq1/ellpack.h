#pragma once
#include <boost/mpi.hpp>

#include <iostream>
#include <ostream>
#include <vector>

namespace mpi = boost::mpi;
template <typename T> class ELLpack {
  private:
    int rows_;
    int skinny_cols_;
    int size_total_;
    int size_rank_;
    int width_;
    int height_;

  public:
    // MPI environment
    mpi::environment env;
    mpi::communicator world;
    int np = world.size();
    int rank = world.rank();

    // Matrices and vectors
    std::vector<T> i_mat;
    std::vector<T> a_mat;
    std::vector<T> v_old;
    std::vector<T> v_new;
    // std::vector<T> v_old_global;
    // std::vector<T> v_new_global;

    void initialize();
    void initialize_stiffness_matrix();
    void initialize_vectors();
    void update();
    void neighbours(int i);
    void print();
    void print_a();
    void print_v();
    T new_v_val(int id);
    int size_total();
    int size_rank();
    int width();
    int height();

    // ELLpack(int rows)
    // ELLpack(int rows)
    //     : rows_(rows),
    //       skinny_cols_(4),
    //       size_total_(rows * rows * 2),
    //       size_rank_(size_total_ / np),
    //       width_(rows * 2),
    //       height_(rows),
    //       i_mat(size_total_ * skinny_cols_),
    //       a_mat(size_total_ * skinny_cols_),
    //       v_old_local(size_total_),
    //       v_new_local(size_total_) {}

    ELLpack(int rows)
        : rows_(rows),
          skinny_cols_(4),
          size_total_(rows * rows * 2),
          width_(rows * 2),
          height_(rows),
          v_old(size_total_),
          v_new(size_total_) {
        size_rank_ = size_total_ / np;
        i_mat.assign(size_rank_ * skinny_cols_, 0);
        a_mat.assign(size_rank_ * skinny_cols_, 0);
    }

    ~ELLpack() = default;
};
