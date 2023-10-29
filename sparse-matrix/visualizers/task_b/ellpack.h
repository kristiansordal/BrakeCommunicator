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
    int min_id_;
    int max_id_;
    int buffer_ = 0;

  public:
    // MPI environment
    mpi::environment env;
    mpi::communicator world;
    int np = world.size();
    int rank = world.rank();

    // Matrices and vectors
    std::vector<int> i_mat;
    std::vector<T> a_mat;
    std::vector<T> v_old;
    std::vector<T> v_new;
    std::vector<int> separators;
    std::vector<T> non_separators;
    std::vector<int> separator_sizes;
    std::vector<T> separator_values;

    void initialize();
    void initialize_stiffness_matrix();
    void initialize_vectors();
    void update();
    void neighbours(int i);
    void print();
    void print_a();
    void print_v();
    void determine_separators();
    void reorder_separators();
    T new_v_val(int id, int &found_seps, std::vector<std::vector<T>> &send_buffer);
    int size_total();
    int size_rank();
    int width();
    int height();
    int min_id();
    int max_id();
    int buffer_size();

    ELLpack(int rows)
        : rows_(rows),
          skinny_cols_(4),
          size_total_(rows * rows * 2),
          width_(rows * 2),
          height_(rows) {
        size_rank_ = size_total_ / np;
        min_id_ = rank * size_rank_;
        max_id_ = min_id_ + size_rank_ - 1;
        v_new.assign(size_rank_, 0);
        v_old.assign(size_total_, 0);
        i_mat.assign(size_rank_ * skinny_cols_, 0);
        a_mat.assign(size_rank_ * skinny_cols_, 0);
    }

    ~ELLpack() = default;
};
