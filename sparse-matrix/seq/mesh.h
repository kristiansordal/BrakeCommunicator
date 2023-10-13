#pragma once
#include <iostream>
#include <ostream>
#include <vector>

template <typename T> class Mesh {
  private:
    int n_;
    int mesh_rows_;
    int mesh_cols_;

  public:
    std::vector<T> mesh;
    void initialize();
    int size();
    int m_dim();
    int n_dim();

    Mesh(int n)
        : n_(n),
          mesh_rows_(n),
          mesh_cols_(n * 2),
          mesh(std::vector<T>(mesh_rows_ * mesh_cols_)) {}
    ~Mesh() = default;
};
