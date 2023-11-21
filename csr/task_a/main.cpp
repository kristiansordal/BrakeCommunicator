#include "fast_matrix_market/app/triplet.hpp"
#include "matrix.hpp"
#include <boost/mpi.hpp>
#include <fast_matrix_market/fast_matrix_market.hpp>
#include <fstream>

using namespace std;
namespace ffm = fast_matrix_market;

void read_file(string path, Matrix &matrix) {
    ifstream file;
    file.open(path);

    if (!file.is_open()) {
        cerr << "Error: Could not open file." << endl;
        return;
    }

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

    int np = world.size();
    int rank = world.rank();
    int ops = 0;

    // Rows per rank
    vector<int> rc;
    rc.assign(np, 0);

    ttots = time.elapsed();

    if (rank == 0) {
        tfiles = time.elapsed();
        read_file(argc[1], M);
        M.init_row_ptr();
        M.init_row_size();
        tfilee = time.elapsed();

        // Load balancing
        double avg_load = (double)M.nnz / (double)np;
        cout << "AVG LOAD: " << avg_load << endl;
        int core = 0;

        for (int i = 0; i < M.nrows; i++) {

            rc[core]++;

            if (M.row_ptr[i + 1] > avg_load * (core + 1)) {
                cout << "CORE: " << core << " HAS " << rc[core] << endl;
                core++;
            }
        }

        for (int i = 1; i < rc.size(); i++) {
            world.send(i, i, rc[i]);
        }

        M.n = rc[0];
    }

    for (int i = 1; i < np; i++) {
        if (rank == i) {
            world.recv(0, i, M.n);
        }
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

        for (int i = 1; i < np; i++) {
            world.send(i, i, rb[i]);
            world.send(i, i, cb[i]);
            world.send(i, i, vb[i]);
        }
    }

    for (int i = 1; i < np; i++) {
        if (rank == i) {
            world.recv(0, i, rb[i]);
            world.recv(0, i, cb[i]);
            world.recv(0, i, vb[i]);

            M.row_ptr = rb[i];
            M.col_ptr = cb[i];
            M.vals = vb[i];
        }
    }

    M.init_v_new();

    world.barrier();
    for (int i = 0; i < 1; i++) {
        M.update(world, time, rank, ops, tcomp, tcomm);
    }

    ttote = time.elapsed();

    cout << "Rank " << rank << " did " << ops << " operations" << endl;
    if (rank == 0) {
        double sum = 0;
        for (auto &i : M.v_old) {
            sum += abs(i * i);
        }
        cout << endl;
        cout << "File:    " << tfilee - tfiles << endl;
        cout << "Comp:    " << tcomp << endl;
        cout << "Comm:    " << tcomm << endl;
        cout << "Total:   " << ttote - ttots << endl;
        cout << "L2 norm: " << sqrt(sum) << endl;
    }

    return 0;
}