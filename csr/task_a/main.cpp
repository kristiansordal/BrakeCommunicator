#include "fast_matrix_market/app/triplet.hpp"
#include "matrix.hpp"
#include <boost/mpi.hpp>
#include <fast_matrix_market/fast_matrix_market.hpp>
#include <fstream>
#include <metis.h>

using namespace std;
namespace mpi = boost::mpi;
namespace ffm = fast_matrix_market;

// TODO: Implement hybrid version (OpenMP + MPI version) of Compressed Sparse Row sparse
// TODO: Matrix-vector multiplication using  1d or 1e strategy with file  reading.
// TODO: Implement file reading.
// TODO: Use metis for load balancing (PartKwayGraph)
// TODO: Check out https://sparcityeu.github.io/sparsebase/pages/getting_started/ for file reading

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

    int np = world.size();
    int rank = world.rank();

    // process mtx file on rank 0
    if (rank == 0) {
        read_file("matrices/Hamrle1.mtx", matrix);
        matrix.init_col_ptr();
    }

    mpi::broadcast(world, matrix.n, 0);
    int rank_size = matrix.n / np;

    // TODO : Currently implemented with files being row major, change to column major
    vector<vector<i64>> col_buffer;
    vector<vector<i64>> row_buffer;
    vector<vector<double>> val_buffer;

    col_buffer.assign(np, vector<i64>());
    row_buffer.assign(np, vector<i64>());
    val_buffer.assign(np, vector<double>());

    if (rank == 0) {
        for (int i = 0; i < np; i++) {
            for (int j = 0; j < rank_size + 1; j++) {
                col_buffer[i].push_back(matrix.col_ptr[i * rank_size + j]);
            }
        }

        for (int i = 0; i < np; i++) {
            for (int j = i == 0 ? 0 : col_buffer[i - 1][col_buffer[i].size() - 1];
                 j < col_buffer[i][col_buffer[i].size() - 1]; j++) {
                row_buffer[i].push_back(matrix.row_ptr[j]);
                val_buffer[i].push_back(matrix.vals[j]);
            }
        }

        for (int i = 0; i < np; i++) {
            world.send(i, i, col_buffer[i]);
            world.send(i, i, row_buffer[i]);
            world.send(i, i, val_buffer[i]);
        }
    }

    for (int i = 0; i < np; i++) {
        if (rank == i) {
            world.recv(0, i, col_buffer[i]);
            world.recv(0, i, row_buffer[i]);
            world.recv(0, i, val_buffer[i]);
            matrix.col_ptr = col_buffer[i];
            matrix.row_ptr = row_buffer[i];
            matrix.vals = val_buffer[i];
        }
    }

    cout << "Rank: " << rank << endl;
    for (auto &i : matrix.col_ptr) {
        cout << i << " ";
    }
    cout << endl;
    for (auto &i : matrix.vals) {
        cout << i << " ";
    }
    cout << endl;

    return 0;
}
