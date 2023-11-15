#include "fast_matrix_market/app/triplet.hpp"
#include "matrix.hpp"
#include <boost/mpi.hpp>
#include <fast_matrix_market/fast_matrix_market.hpp>
#include <fstream>
#include <metis.h>

using namespace std;
namespace mpi = boost::mpi;
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

    ffm::read_matrix_market_triplet(file, matrix.nrows, matrix.ncols, matrix.row_ptr, matrix.col_ptr, matrix.vals);
    matrix.nnz = matrix.vals.size();
    matrix.n = matrix.nrows;

    cout << "Successfully read file " << file_name << endl;
    file.close();
}

int main() {
    Matrix matrix;

    mpi::environment env;
    mpi::communicator world;
    mpi::timer time;
    double time_total_start;
    double time_total_end;
    double time_file_start;
    double time_file_end;
    double time_comm_start;
    double time_comm_end;
    double time_comp_start;
    double time_comp_end;

    int np = world.size();
    int rank = world.rank();

    time_total_start = time.elapsed();
    // process mtx file on rank 0
    if (rank == 0) {
        time_file_start = time.elapsed();
        read_file("matrices/cage15.mtx", matrix);
        matrix.init_col_ptr();
        time_file_end = time.elapsed();
    }

    mpi::broadcast(world, matrix.n, 0);
    matrix.init_v();
    matrix.n /= np;

    vector<vector<i64>> cb;
    vector<vector<i64>> rb;
    vector<vector<double>> vb;

    cb.assign(np, vector<i64>());
    rb.assign(np, vector<i64>());
    vb.assign(np, vector<double>());

    if (rank == 0) {
        for (int i = 0; i < np; i++) {
            for (int j = 0; j < matrix.n + 1; j++) {
                cb[i].push_back(matrix.col_ptr[i * matrix.n + j]);
            }
        }

        for (int i = 0; i < np; i++) {
            for (int j = i == 0 ? 0 : cb[i - 1][cb[i - 1].size() - 1]; j < cb[i][cb[i].size() - 1]; j++) {
                rb[i].push_back(matrix.row_ptr[j]);
                vb[i].push_back(matrix.vals[j]);
            }
        }
        matrix.col_ptr = cb[0];
        matrix.row_ptr = rb[0];
        matrix.vals = vb[0];

        for (int i = 1; i < np; i++) {
            cout << cb[i].size() * sizeof(i64) << endl;
            cout << rb[i].size() * sizeof(i64) << endl;
            cout << vb[i].size() * sizeof(double) << endl;

            world.send(i, i, cb[i]);
            world.send(i, i, rb[i]);
            world.send(i, i, vb[i]);
        }
    }

    for (int i = 1; i < np; i++) {
        if (rank == i) {
            world.recv(0, i, cb[i]);
            world.recv(0, i, rb[i]);
            world.recv(0, i, vb[i]);

            matrix.col_ptr = cb[i];
            matrix.row_ptr = rb[i];
            matrix.vals = vb[i];
        }
    }

    time_comp_start = time.elapsed();

    for (int i = 0; i < 5; i++) {
        matrix.update(rank);
    }

    time_comp_end = time.elapsed();

    std::vector<double> vv;
    mpi::gather(world, matrix.v_old.data(), matrix.v_old.size(), vv, 0);

    time_total_end = time.elapsed();

    if (rank == 0) {

        double sum = 0;
        for (auto &i : vv) {
            sum += abs(i * i);
        }

        cout << "Comp:    " << time_comp_end - time_comp_start << endl;
        cout << "Total:   " << time_total_end - time_total_start << endl;
        cout << "L2 norm: " << sqrt(sum) << endl;
    }

    return 0;
}
