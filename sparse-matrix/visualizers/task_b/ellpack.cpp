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

template <typename T> void ELLpack<T>::initialize_vectors() {
    if (rank == 0) {
        for (int i = 0; i < size_total(); i++) {
            v_old[i] = 0;
        }
        v_new[1] = 1;
        v_new[3] = 1;
        v_new[5] = 1;
        v_new[7] = 1;
        v_old[1] = 1;
        v_old[3] = 1;
        v_old[5] = 1;
        v_old[7] = 1;
    }

    mpi::broadcast(world, v_old.data(), size_total(), 0);
}

template <typename T> void ELLpack<T>::initialize_stiffness_matrix() {
    for (int i = 0; i < size_rank() * skinny_cols_; i += skinny_cols_) {
        // The resitance of the spread of the signal, should sum to 1
        a_mat[i] = 0.2;
        a_mat[i + 1] = 0.3;
        a_mat[i + 2] = 0.3;
        a_mat[i + 3] = 0.2;
    }
}

// Determines at which index in the i_mat matrix the separators are stored
// Populates the vectors separators and non_separators.
template <typename T> void ELLpack<T>::determine_separators() {
    for (int i = 0; i < size_rank() * skinny_cols_; i += skinny_cols_) {
        for (int j = 0; j < skinny_cols_; j++) {
            if (rank == 0) {
            }
            int n = i_mat[i + j];

            if (n == -1) {
                continue;
            }

            if (n > max_id() || n < min_id()) {
                separators.push_back(i);
            }
        }
    }

    int s = (int)separators.size();
    mpi::all_gather(world, s, separator_sizes);

    for (int i = 0; i < size_rank() * skinny_cols_; i += skinny_cols_) {
        if (std::find(separators.begin(), separators.end(), i) == separators.end()) {
            non_separators.push_back(i);
        }
    }
}

// Reorders the i_mat matrix to store the separators at the first n indices,
// given n separators.
template <typename T> void ELLpack<T>::reorder_separators() {
    std::vector<T> ordered;
    ordered.resize(i_mat.size());
    auto s = (int)separators.size();
    auto n = (int)non_separators.size();
    auto offset = s * skinny_cols_;

    for (int i = 0; i < s; i++) {
        for (int j = 0; j < skinny_cols_; j++) {
            ordered[i * skinny_cols_ + j] = i_mat[separators[i] + j];
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < skinny_cols_; j++) {
            ordered[i * skinny_cols_ + j + offset] = i_mat[non_separators[i] + j];
        }
    }

    std::swap(i_mat, ordered);
    if (rank == 0) {
        for (int i = 0; i < 4; i++) {
            std::cout << v_old[i_mat[4 * i]] << std::endl;
        }
    }
}

template <typename T> void ELLpack<T>::update() {
    std::vector<std::vector<T>> send_buffer;
    send_buffer.assign(np, std::vector<T>());

    for (int i = 0; i < np; i++) {
        if (i == rank) {
            for (int j = 0; j < separator_sizes[i]; j++) {
                send_buffer[i].push_back(v_new[i_mat[j * skinny_cols_]]);
            }
        }
    }

    for (int dest = 0; dest < np; dest++) {
        mpi::broadcast(world, send_buffer[dest].data(), send_buffer[dest].size(), dest);
    }

    if (rank == 1) {
        for (int i = 0; i < (int)send_buffer.size(); i++) {
            std::cout << i << ": ";
            for (int j = 0; j < (int)send_buffer[i].size(); j++) {
                std::cout << send_buffer[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    int found_seps = 0;
    for (int i = 0; i < size_rank(); i++) {
        v_new[i] = new_v_val(i, found_seps, send_buffer);
    }

    world.barrier();
    mpi::all_gather(world, v_new.data(), size_rank(), v_old);
}

template <typename T> T ELLpack<T>::new_v_val(int id, int &found_seps, std::vector<std::vector<T>> &send_buffer) {
    std::vector<double> v_vals;
    int s = id * skinny_cols_;
    bool found_sep = false;

    for (int i = 0; i < skinny_cols_; i++) {
        if (id < separator_sizes[rank] && found_seps < separator_sizes[rank]) {
            if (i_mat[s + i] > 0 && (i_mat[s + i] < min_id() || i_mat[s + i] > max_id())) {
                if (rank == 1) {
                    // std::cout << "Separator: " << i_mat[s + i] << std::endl;
                    // std::cout << send_buffer[rank][found_seps] << std::endl;
                    // std::cout << found_seps << std::endl;
                }
                found_sep = true;
                v_vals.push_back(send_buffer[rank][found_seps++]);
            }
        }
        if (!found_sep) {
            v_vals.push_back(i_mat[s + i] != -1 ? v_old[i_mat[s + i]] : v_old[i_mat[s]]);
        }
    }

    return a_mat[s] * v_vals[0] + a_mat[s + 1] * v_vals[1] + a_mat[s + 2] * v_vals[2] + a_mat[s + 3] * v_vals[3];
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
    for (int i = 0; i < (int)std::size(v_old); i++) {
        if (i % width() == 0 && i > 0) {
            std::cout << std::endl;
        }
        std::cout << v_old[i] << " ";
    }
}

template <typename T> int ELLpack<T>::size_total() { return size_total_; }
template <typename T> int ELLpack<T>::size_rank() { return size_rank_; }
template <typename T> int ELLpack<T>::width() { return width_; }
template <typename T> int ELLpack<T>::height() { return height_; }
template <typename T> int ELLpack<T>::min_id() { return min_id_; }
template <typename T> int ELLpack<T>::max_id() { return max_id_; }
template <typename T> int ELLpack<T>::buffer_size() { return buffer_; }
