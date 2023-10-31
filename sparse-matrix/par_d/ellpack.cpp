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
            v_new[i % size_rank()] = v_old[i];
        }

        v_old[0] = 1;
        v_new[0] = 1;
    }

    mpi::broadcast(world, v_new.data(), size_rank(), 0);
}

template <typename T> void ELLpack<T>::initialize_stiffness_matrix() {
    for (int i = 0; i < size_rank() * skinny_cols_; i += skinny_cols_) {
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

            int n = i_mat[i + j];

            if (n == -1) {
                continue;
            }

            if (n < min_id()) {
                int dest = rank - 1;

                send_list[rank][dest].push_back(i_mat[i]);
                send_list[dest][rank].push_back(n);

            } else if (n > max_id()) {
                int dest = rank + 1;

                send_list[rank][dest].push_back(i_mat[i]);
                send_list[dest][rank].push_back(n);
            }
        }
    }
}

template <typename T> void ELLpack<T>::update() {
    std::vector<std::vector<T>> send_buffer;
    std::vector<std::vector<T>> recv_buffer;
    send_buffer.assign(np, std::vector<T>());
    recv_buffer.assign(np, std::vector<T>());

    for (int i = 0; i < np; i++) {
        if (i != rank && (int)send_list[rank][i].size() != 0) {
            if (v_new.size() > 0) {
                for (int j = 0; j < (int)send_list[rank][i].size(); j++) {
                    send_buffer[i].push_back(v_new[send_list[rank][i][j] % size_rank()]);
                }
            } else {
                for (int j = 0; j < (int)send_list[rank][i].size(); j++) {
                    send_buffer[i].push_back(v_old[send_list[rank][i][j]]);
                }
            }
        }
    }

    std::vector<mpi::request> send_requests;
    std::vector<mpi::request> recv_requests;

    int t1 = time.elapsed();
    for (int dest = 0; dest < np; dest++) {
        if (dest == rank) {
            continue;
        }

        if (send_list[rank][dest].size() == 0) {
            continue;
        }

        std::cout << "rank: " << rank << " waiting" << std::endl;
        mpi::request send_request = world.isend(dest, 0, send_buffer[dest]);
        mpi::request recv_request = world.irecv(dest, 0, recv_buffer[dest]);
        std::cout << "rank: " << rank << " done waiting" << std::endl;

        send_requests.push_back(send_request);
        recv_requests.push_back(recv_request);
    }

    // std::cout << "waiting" << std::endl;
    // mpi::wait_all(send_requests.begin(), send_requests.end());
    mpi::wait_all(recv_requests.begin(), recv_requests.end());
    // std::cout << "done waiting" << std::endl;
    comm_time += time.elapsed() - t1;

    for (int i = 0; i < size_rank(); i++) {
        v_new[i] = new_v_val(i, recv_buffer);
    }
}

template <typename T> T ELLpack<T>::new_v_val(int id, std::vector<std::vector<T>> &recv_buffer) {
    std::vector<double> v_vals;
    int s = id * skinny_cols_;
    int n1 = rank + 1;
    int n2 = rank - 1;

    for (int i = 0; i < skinny_cols_; i++) {
        int cell = i_mat[s + i];
        bool found_sep = false;

        if (n1 < np) {
            auto sendv_list = send_list[n1][rank];
            auto recv_list = send_list[rank][n1];

            if (sendv_list.size() > 0 && recv_list.size() > 0) {
                for (int j = 0; j < (int)sendv_list.size(); j++) {
                    if (sendv_list[j] == cell) {
                        found_sep = true;
                        v_vals.push_back(recv_buffer[n1][j]);
                    }
                }
            }
        }
        if (n2 >= 0) {
            auto recv_list = send_list[n2][rank];
            auto sendv_list = send_list[rank][n2];

            if (sendv_list.size() > 0 && recv_list.size() > 0) {
                for (int j = 0; j < (int)recv_list.size(); j++) {
                    if (recv_list[j] == cell) {
                        found_sep = true;
                        v_vals.push_back(recv_buffer[n2][j]);
                    }
                }
            }
        }

        if (!found_sep) {
            v_vals.push_back(i_mat[s + i] != -1 ? v_new[i_mat[s + i] % size_rank()] : v_new[i_mat[s] % size_rank()]);
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
