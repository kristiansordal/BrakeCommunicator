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
}

template <typename T> void ELLpack<T>::initialize_vectors() {
    if (rank == 0) {
        for (int i = 0; i < size_total(); i++) {
            v_old[i] = 0;
        }

        v_old[1] = 1;
        v_old[2] = 2;
        v_old[3] = 3;
        v_old[4] = 4;
        v_old[5] = 5;
        v_old[6] = 6;
        v_old[7] = 7;
        v_old[8] = 8;
        v_old[9] = 9;
        v_old[10] = 10;
        v_old[11] = 11;
        v_old[12] = 12;
        v_old[13] = 13;
        v_old[14] = 14;
        v_old[15] = 15;
        v_old[16] = 16;
        v_old[17] = 17;
        v_old[18] = 18;
    }

    mpi::broadcast(world, v_old.data(), size_total(), 0);
}

// Should first send the separators, i.e. the values in v_old
template <typename T> void ELLpack<T>::update() {

    for (int i = 0; i < (int)separator_sizes.size(); i++) {
        std::vector<T> vals;
        vals.assign(separator_sizes[i], 0);

        for (int j = 0; j < separator_sizes[i]; j++) {
            vals[j] = v_old[i_mat[j * skinny_cols_]];
        }
        // each rank now has its respective vals vector
        // here is where rank i should broadcast it i guess

        std::cout << "RANK: " << rank << " " << vals.size() << std::endl;
        mpi::all_gather(world, vals.data(), separator_sizes[i], separator_values);
    }

    // The offset at which to get the v_old values
    int offset = std::accumulate(separator_sizes.begin(), separator_sizes.begin() + rank, 0);

    world.barrier();
    if (rank == 1) {
        for (auto &v : separator_values) {
            std::cout << v << " ";
        }
    }
    std::cout << std::endl;

    // for (auto &i : separators) {
    //     mpi::all_gather(world, v_old[i_mat[i]], &(v_old[i_mat[i]]));
    // }

    // for (int i = 0; i < (int)separator_sizes.size(); i++) {
    //     std::cout << separator_sizes[i] << std::endl;
    // }

    for (int i = 0; i < size_rank(); i++) {
        v_old[i] = new_v_val(i, offset);
    }
    // mpi::all_gather(world, v_new.data(), size_rank(), v_old);
}

template <typename T> T ELLpack<T>::new_v_val(int id, int offset) {
    // double v1, v2, v3, v4;
    std::vector<double> v_vals;
    int s = id * skinny_cols_;

    for (int i = 0; i < skinny_cols_; i++) {
        s += i;
        bool is_separator = std::find(separators.begin(), separators.end(), s) != separators.end();
        // std::cout << "S: " << s << " is separator: " << is_separator << std::endl;
        if (is_separator) {
            v_vals.push_back(separator_values[offset]);
        } else {
            if (i == 0) {
                v_vals.push_back(v_old[i_mat[s]]);
            }
            v_vals.push_back(i_mat[s + i] != -1 ? v_old[i_mat[s + i]] : v_old[i_mat[s]]);
        }
    }

    // v1 = v_old[i_mat[s]];
    // v2 = i_mat[s + 1] != -1 ? v_old[i_mat[s + 1]] : v_old[i_mat[s]];
    // v3 = i_mat[s + 2] != -1 ? v_old[i_mat[s + 2]] : v_old[i_mat[s]];
    // v4 = i_mat[s + 3] != -1 ? v_old[i_mat[s + 3]] : v_old[i_mat[s]];

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
