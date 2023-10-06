#pragma once
#include <iostream>
#include <ostream>
#include <vector>

template <typename T>
class Mesh {
   public:
    std::vector<T> mesh;
    void initialize() {}

   private:
    int n_;
    int np_;

    Mesh(int n, int np)
        : mesh(std::vector<T>(n / np)),
          n_(n),
          np_(np) {}
    ~Mesh() = default;
};
