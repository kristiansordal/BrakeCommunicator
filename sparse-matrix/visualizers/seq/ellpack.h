#pragma once

#include <iostream>
#include <ostream>
#include <vector>

template <typename T> class ELLpack {
  private:
    int rows_;
    int skinny_cols_;
    int size_;
    int width_;
    int height_;

  public:
    // Matrices and vectors
    std::vector<T> i_mat;
    std::vector<T> a_mat;
    std::vector<T> v_old;
    std::vector<T> v_new;

    void initialize();
    void initialize_stiffness_matrix();
    void initialize_vectors();
    void update();
    void neighbours(int i);
    void print();
    void print_a();
    void print_v();
    T new_v_val(int id);
    int size();
    int width();
    int height();

    ELLpack(int rows)
        : rows_(rows),
          skinny_cols_(4),
          size_(rows * rows * 2),
          width_(rows * 2),
          height_(rows),
          v_old(size_),
          v_new(size_) {
        i_mat.assign(size_ * skinny_cols_, 0);
        a_mat.assign(size_ * skinny_cols_, 0);
    }

    ~ELLpack() = default;
};
