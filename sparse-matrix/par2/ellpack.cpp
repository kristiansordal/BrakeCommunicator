#include "ellpack.h"

template <typename T> void ELLpack<T>::initialize() {
    for (int i = 0; i < size_rank(); i++) {
        neighbours(i);
    }
}

template <typename T> void ELLpack<T>::neighbours(int i) {
    int mesh_cols = width();
    int id = i + rank * size_rank();
    i_mat[i * skinny_cols_] = id;
    i_mat[i * skinny_cols_ + 1] = (id % mesh_cols) - 1 >= 0 ? id - 1 : -1;
    i_mat[i * skinny_cols_ + 2] = (id % mesh_cols) + 1 >= mesh_cols ? -1 : id + 1;

    if (i % 2 == 0) {
        i_mat[i * skinny_cols_ + 3] = ((id - mesh_cols) < 0 ? -1 : id - mesh_cols + 1);
    } else {
        i_mat[i * skinny_cols_ + 3] = ((id + mesh_cols) > size_total() ? -1 : id + mesh_cols - 1);
    }
}

template <typename T> void ELLpack<T>::initialize_stiffness_matrix() {
    for (int i = 0; i < size_rank() * skinny_cols_; i += skinny_cols_) {
        a_mat[i] = 0.3;
        a_mat[i + 1] = 0.2;
        a_mat[i + 2] = 0.2;
        a_mat[i + 3] = 0.3;
    }
}

template <typename T> void ELLpack<T>::initialize_vectors() {
    if (rank == 0) {
        for (int i = 0; i < size_total(); i++) {
            v_old[i] = 0;
        }

        v_old[0] = 0.2;
    }

    mpi::broadcast(world, v_old.data(), size_total(), 0);
}

template <typename T> void ELLpack<T>::update() {
    for (int i = 0; i < size_rank(); i++) {
        v_new[i] = new_v_val(i);
    }

    mpi::all_gather(world, v_new.data(), size_rank(), v_old);
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
        std::cout << i_mat[i] << " ";
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

template <typename T> int ELLpack<T>::size_total() { return size_rank_; }
template <typename T> int ELLpack<T>::size_rank() { return size_rank_; }
template <typename T> int ELLpack<T>::width() { return width_; }
template <typename T> int ELLpack<T>::height() { return height_; }
