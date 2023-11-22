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
    i64 ops = 0;

    // Rows per rank
    // vector<int> rc;
    // rc.assign(np, 0);

    ttots = time.elapsed();

    if (rank == 0) {
        tfiles = time.elapsed();
        read_file(argc[1], M);
        tfilee = time.elapsed();

        idx_t ncon = 1;
        idx_t nparts = np;
        idx_t objval;
        vector<idx_t> part(M.nnz);
        int rc = METIS_PartGraphKway(&M.nnz, &ncon, M.row_ptr.data(), M.col_ptr.data(), NULL, NULL, NULL, &nparts, NULL,
                                     NULL, NULL, &objval, part.data());

        cout << "XADJ:      ";
        for (auto &i : M.row_ptr) {
            cout << i << " ";
        }
        cout << endl;

        cout << "Adjacency: ";
        for (auto &i : M.col_ptr) {
            cout << i << " ";
        }
        cout << endl;
        cout << "Part:      ";
        for (int i = 0; i < M.nnz; i++) {
            cout << part[i] << " ";
        }
        cout << endl;

        vector<vector<idx_t>> rb;
        vector<vector<idx_t>> cb;
        vector<vector<double>> vb;

        rb.assign(np, vector<idx_t>());
        cb.assign(np, vector<idx_t>());
        vb.assign(np, vector<double>());

        cout << "P size: " << part.size() << endl;
        for (int i = 0; i < part.size(); i++) {
            rb[part[i]].push_back(M.row_ptr[i]);
            cb[part[i]].push_back(M.col_ptr[i]);
            vb[part[i]].push_back(M.vals[i]);
        }

        // print the stuff
        for (auto &i : rb) {
            for (auto &j : i) {
                cout << j << " ";
            }
            cout << endl;
        }
        cout << endl;
        for (auto &i : cb) {
            for (auto &j : i) {
                cout << j << " ";
            }
            cout << endl;
        }
        cout << endl;
        for (auto &i : vb) {
            for (auto &j : i) {
                cout << j << " ";
            }
            cout << endl;
        }
        cout << endl;
    }

    // for (int i = 1; i < np; i++) {
    //     if (rank == i) {
    //         world.recv(0, i, M.n);
    //     }
    // }

    // mpi::broadcast(world, M.nrows, 0);
    // M.init_v_old(np);

    // vector<vector<i64>> rb;
    // vector<vector<i64>> cb;
    // vector<vector<double>> vb;

    // rb.assign(np, vector<i64>());
    // cb.assign(np, vector<i64>());
    // vb.assign(np, vector<double>());

    // if (rank == 0) {
    //     int offset = 0;
    //     for (int i = 0; i < np; i++) {
    //         for (int j = offset; j < rc[i] + 1 + offset; j++) {
    //             rb[i].push_back(M.row_ptr[j]);
    //         }
    //         offset += rc[i];
    //     }

    //     rb[np - 1].push_back(M.row_ptr[M.nrows]);

    //     for (int i = 0; i < np; i++) {
    //         for (int j = i == 0 ? 0 : rb[i - 1][rb[i - 1].size() - 1]; j < rb[i][rb[i].size() - 1]; j++) {
    //             cb[i].push_back(M.col_ptr[j]);
    //             vb[i].push_back(M.vals[j]);
    //         }
    //     }

    //     M.row_ptr = rb[0];
    //     M.col_ptr = cb[0];
    //     M.vals = vb[0];

    //     for (int i = 1; i < np; i++) {
    //         world.send(i, i, rb[i]);
    //         world.send(i, i, cb[i]);
    //         world.send(i, i, vb[i]);
    //     }
    // }

    // for (int i = 1; i < np; i++) {
    //     if (rank == i) {
    //         world.recv(0, i, rb[i]);
    //         world.recv(0, i, cb[i]);
    //         world.recv(0, i, vb[i]);

    //         M.row_ptr = rb[i];
    //         M.col_ptr = cb[i];
    //         M.vals = vb[i];
    //     }
    // }

    // M.init_v_new();

    // world.barrier();
    // for (int i = 0; i < 100; i++) {
    //     M.update(world, time, rank, ops, tcomp, tcomm);
    // }

    // ttote = time.elapsed();
    // vector<i64> ops_all;
    // mpi::gather(world, ops, ops_all, 0);

    // if (rank == 0) {
    //     double sum = 0;

    //     for (auto &i : M.v_old) {
    //         sum += abs(i * i);
    //     }

    //     cout << endl;
    //     cout << "File:    " << tfilee - tfiles << endl;
    //     cout << "Comp:    " << tcomp << endl;
    //     cout << "Comm:    " << tcomm << endl;
    //     cout << "Total:   " << ttote - ttots << endl;
    //     cout << "L2 norm: " << sqrt(sum) << endl;
    //     for (int i = 0; i < ops_all.size(); i++) {
    //         cout << "Rank " << i << " ops: " << ops_all[i] << endl;
    //     }
    // }

    return 0;
}
