#include "fast_matrix_market/app/triplet.hpp"
#include "matrix.hpp"
#include <boost/mpi.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <fast_matrix_market/fast_matrix_market.hpp>
#include <fstream>

using namespace std;
namespace ffm = fast_matrix_market;

void read_file(string path, Matrix &matrix) {
    ifstream file;
    file.open(path);

    cout << "Opening file..." << endl;

    if (!file.is_open()) {
        cout << "Error: Could not open file." << endl;
        return;
    }

    cout << "File successfully opened" << endl;

    auto get_file_name = [](const string &path) {
        size_t found = path.find_last_of("/");
        return (found != string::npos) ? path.substr(found + 1) : path;
    };

    string file_name = get_file_name(path);

    ffm::read_matrix_market_triplet(file, matrix.nrows, matrix.ncols, matrix.col_ptr, matrix.row_ptr, matrix.vals);
    matrix.nnz = matrix.vals.size();
    matrix.n = matrix.nrows;

    cout << "Successfully read file " << file_name << endl;
    file.close();
}

int main(int argv, char **argc) {
    Matrix M;

    mpi::environment env;
    mpi::communicator world;
    mpi::timer time;

    double ttots;
    double ttote;
    double tfiles;
    double tfilee;
    double tcomm;
    double tcomp;
    double tcomp_kernel;

    int np = world.size();
    int rank = world.rank();
    ulli ops = 0;

    vector<i64> rc;
    rc.assign(np, 0);

    ttots = time.elapsed();

    if (rank == 0) {
        cout << "SIZE:    " << np << endl;
        tfiles = time.elapsed();
        read_file(argc[1], M);
        M.init_row_ptr();
        M.init_row_size();
        tfilee = time.elapsed();

        // Load balancing
        int avg_load = M.nnz / np;
        int core = 0;

        cout << "Performing Load Balancing" << endl;
        for (int i = 0; i < M.nrows; i++) {
            rc[core]++;

            if (M.row_ptr[i + 1] > avg_load * (core + 1)) {
                core++;
            }
        }
        cout << "Load balancing completed" << endl;

        for (int i = 1; i < np; i++) {
            cout << "Sending: " << rc[i] << " to " << i << endl;
            world.send(i, i, rc[i]);
            cout << "Done sending: " << rc[i] << " to " << i << endl;
        }

        M.n = rc[0];
    }else{
    cout << "Rank " << rank << " is waiting for recieve" << endl;
    world.recv(0, rank, rc[rank]);
    M.n = rc[rank];
    cout << "Rank " << rank << " has recieved, got: " << M.n << endl;
    }

    mpi::broadcast(world, M.nrows, 0);
    M.init_v_old(np);

    vector<vector<i64>> rb;
    vector<vector<i64>> cb;
    vector<vector<double>> vb;

    rb.assign(np, vector<i64>());
    cb.assign(np, vector<i64>());
    vb.assign(np, vector<double>());

    if (rank == 0) {
        int offset = 0;
        for (int i = 0; i < np; i++) {
            for (int j = offset; j < rc[i] + 1 + offset; j++) {
                rb[i].push_back(M.row_ptr[j]);
            }
            offset += rc[i];
        }

        rb[np - 1].push_back(M.row_ptr[M.nrows]);

        for (int i = 0; i < np; i++) {
            for (int j = i == 0 ? 0 : rb[i - 1][rb[i - 1].size() - 1]; j < rb[i][rb[i].size() - 1]; j++) {
                cb[i].push_back(M.col_ptr[j]);
                vb[i].push_back(M.vals[j]);
            }
        }

        M.row_ptr = rb[0];
        M.col_ptr = cb[0];
        M.vals = vb[0];

        double t1 = time.elapsed();
        for (int i = 1; i < np; i++) {
            world.send(i, i, rb[i]);
            world.send(i, i, cb[i]);
            world.send(i, i, vb[i]);
        }
        tcomm += time.elapsed() - t1;
    }

    if (rank != 0) {
        world.recv(0, rank, rb[rank]);
        world.recv(0, rank, cb[rank]);
        world.recv(0, rank, vb[rank]);

        M.row_ptr = rb[rank];
        M.col_ptr = cb[rank];
        M.vals = vb[rank];
    }

    M.init_v_new();

    world.barrier();
    tcomp = time.elapsed();
    for (int i = 0; i < 100; i++) {
        M.update(world, time, rank, tcomp_kernel, tcomm);
    }
    world.barrier();
    tcomp = time.elapsed() - tcomp;
    ttote = time.elapsed();

    if (rank == 0) {
        double sum = 0;
        ulli ops = (ulli)M.nnz * 2 * 100;

        for (auto &i : M.v_old) {
            sum += abs(i * i);
        }

        cout << endl;
        cout << "File:             " << tfilee - tfiles << endl;
        cout << "Comp:             " << tcomp << endl;
        cout << "Comm:             " << tcomm << endl;
        cout << "Total:            " << ttote - ttots << endl;
        cout << "L2 norm:          " << sqrt(sum) << endl;
        cout << "OPS:              " << ops << endl;
        cout << "GFLOPS:           " << ops / (tcomp * 1e9) << endl;
        cout << "GFLOPS:           " << ops / tcomp / 1e9 << endl;
        cout << "GFLOPS (kernel):  " << ops / (tcomp_kernel * np * 1e9) << endl;
    }

    return 0;
}
