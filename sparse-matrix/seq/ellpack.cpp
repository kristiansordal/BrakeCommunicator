#pragma once
#include "ellpack.h"
#include "mesh.cpp"
#include "mesh.h"

template <typename T> void ELLpack<T>::initialize() {
    for (int i = 0; i < size(); i++) {
        neighbours(i);
    }
}

template <typename T> void ELLpack<T>::neighbours(int i) {
    int mesh_cols = width_;
    i_mat[i * skinny_cols_] = i;
    i_mat[i * skinny_cols_ + 1] = (i % mesh_cols) - 1 >= 0 ? i - 1 : -1;
    i_mat[i * skinny_cols_ + 2] = (i % mesh_cols) + 1 >= mesh_cols ? -1 : i + 1;

    if (i % 2 == 0) {
        i_mat[i * skinny_cols_ + 3] = ((i - mesh_cols) < 0 ? -1 : i - mesh_cols + 1);
    } else {
        i_mat[i * skinny_cols_ + 3] = ((i + mesh_cols) > size() ? -1 : i + mesh_cols - 1);
    }
}

template <typename T> void ELLpack<T>::initialize_stiffness_matrix() {
    for (int i = 0; i < size() * skinny_cols_; i += skinny_cols_) {
        a_mat[i] = 0.3;
        a_mat[i + 1] = 0.2;
        a_mat[i + 2] = 0.2;
        a_mat[i + 3] = 0.3;
    }
}

template <typename T> void ELLpack<T>::initialize_vectors() {
    v_old[0] = 0.2;
    for (int i = 1; i < size(); i++) {
        v_old[i] = 0;
        v_new[i] = 0;
    }

    v_old[220] = 0.9;
}

template <typename T> void ELLpack<T>::update() {
    for (int i = 0; i < size(); i++) {
        v_new[i] = new_v_val(i);
    }
    std::swap(v_old, v_new);
}

template <typename T> T ELLpack<T>::new_v_val(int id) {
    double v1, v2, v3, v4;
    int s = id * skinny_cols_;

    v1 = v_old[i_mat[s]];
    v2 = i_mat[s + 1] != -1 ? v_old[i_mat[s + 1]] : v_old[i_mat[s]];
    v3 = i_mat[s + 2] != -1 ? v_old[i_mat[s + 2]] : v_old[i_mat[s]];
    v4 = i_mat[s + 3] != -1 ? v_old[i_mat[s + 3]] : v_old[i_mat[s]];

    return a_mat[s] * v1 + a_mat[s + 1] * v2 + a_mat[s + 2] * v3 + a_mat[s + 3] * v4;
}

template <typename T> void ELLpack<T>::print() {
    for (int i = 0; i < (int)std::size(i_mat); i++) {
        if (i % skinny_cols_ == 0 && i > 0) {
            std::cout << std::endl;
        }
        std::cout << "(" << i_mat[i] << ", " << v_new[i_mat[i]] << ") ";
    }
    std::cout << std::endl;
}
template <typename T> void ELLpack<T>::print_v() {
    for (int i = 0; i < (int)std::size(v_new); i++) {
        if (i % width() == 0 && i > 0) {
            std::cout << std::endl;
        }
        std::cout << v_new[i] << " ";
    }
}

template <typename T> int ELLpack<T>::size() { return size_; }
template <typename T> int ELLpack<T>::width() { return width_; }
template <typename T> int ELLpack<T>::height() { return height_; }
