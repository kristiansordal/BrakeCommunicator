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
    // std::cout << "R: " << rank << std::endl;
    for (int i = 0; i < size_rank() * skinny_cols_; i += skinny_cols_) {
        for (int j = 0; j < skinny_cols_; j++) {

            int n = i_mat[i + j];

            if (n == -1) {
                continue;
            }

            if (n < min_id()) {
                int dest = rank - 1;

                send_list[rank][dest].push_back(i_mat[i]);
                send_list[dest][rank].push_back(n);

                // std::cout << "(" << rank - 1 << ", " << rank << "): " << n << std::endl;
                // std::cout << "(" << rank << ", " << rank - 1 << "): " << i_mat[i] << std::endl;
            } else if (n > max_id()) {
                int dest = rank + 1;

                send_list[dest][rank].push_back(n);
                send_list[rank][dest].push_back(i_mat[i]);

                // std::cout << "(" << dest << ", " << rank << "): " << n << std::endl;
                // std::cout << "(" << rank << ", " << dest << "): " << i_mat[i] << std::endl;
            }
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

template <typename T> void ELLpack<T>::update() {
    std::vector<std::vector<T>> send_buffer;
    send_buffer.assign(np, std::vector<T>());
    world.barrier();

    for (int i = 0; i < np; i++) {
        if (i != rank) {
            if ((int)send_list[i][rank].size() != 0) {
                for (int j = 0; j < (int)send_list[i][rank].size(); j++) {
                    send_buffer[i].push_back(v_old[send_list[i][rank][j]]);
                }
            }
        }
    }

    // for (int i = 0; i < np; i++) {
    //     if (i != rank && (int)send_list[i][rank].size() != 0) {
    //         world.isend(i, 0, send_buffer[i].data(), send_buffer[i].size());
    //     } // else if (i == rank && (int)send_list[i][rank].size() != 0) {
    //       //  world.irecv(i, 0, send_buffer[i].data(), send_buffer[i].size());
    //     //}
    // }

    // if (rank == 0) {
    //     for (int i = 0; i < (int)send_buffer.size(); i++) {
    //         for (int j = 0; j < (int)send_buffer[i].size(); j++) {
    //             std::cout << send_buffer[i][j] << " ";
    //         }
    //         std::cout << std::endl;
    //     }
    // }

    for (int i = 0; i < size_rank(); i++) {
        v_new[i] = new_v_val(i, send_buffer);
    }
    // world.barrier();
    // mpi::all_gather(world, v_new.data(), size_rank(), v_old);
}

template <typename T> T ELLpack<T>::new_v_val(int id, std::vector<std::vector<T>> &send_buffer) {
    std::vector<double> v_vals;
    int s = id * skinny_cols_;
    std::cout << s << std::endl;
    // int found_seps = 0;

    for (int i = 0; i < skinny_cols_; i++) {
        int n1 = rank + 1;
        int n2 = rank - 1;
        int n3 = send_buffer[0][0];
        std::cout << n3 << "n1: " << n1 << " n2: " << n2 << std::endl;

        if (n1 <= np) {
            auto recv_list = send_list[n1][rank];
            auto sendv_list = send_list[rank][n1];

            auto sep_iter = std::find(sendv_list.begin(), sendv_list.end(), i_mat[s + i]);
            auto is_separator = sep_iter != sendv_list.end();

            if (is_separator) {
                // print the sep_iter id in recv_list

                std::cout << i << "" << std::endl;
            }

            // if (rank == 1) {
            //     std::cout << "rank" << rank << " recv list: ";
            //     for (int i = 0; i < (int)recv_list.size(); i++) {
            //         std::cout << recv_list[i] << " ";
            //     }

            //     std::cout << std::endl;
            //     std::cout << "rank" << rank << " send list: ";
            //     for (int i = 0; i < (int)sendv_list.size(); i++) {
            //         std::cout << sendv_list[i] << " ";
            //     }
            //     std::cout << std::endl;
            // }
        }
        if (n2 >= 0) {
            // auto recv_list = send_list[n2][rank];
            // auto sendv_list = send_list[rank][n2];

            // if (rank == 1) {
            //     std::cout << "rank" << rank << " recv list: ";
            //     for (int i = 0; i < (int)recv_list.size(); i++) {
            //         std::cout << recv_list[i] << " ";
            //     }

            //     std::cout << std::endl;
            //     std::cout << "rank" << rank << " send list: ";
            //     for (int i = 0; i < (int)sendv_list.size(); i++) {
            //         std::cout << sendv_list[i] << " ";
            //     }
            //     std::cout << std::endl;
            // }
        }

        // return a_mat[s] * v_vals[0] + a_mat[s + 1] * v_vals[1] + a_mat[s + 2] * v_vals[2] + a_mat[s + 3] * v_vals[3];
    }
    return 1;
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
