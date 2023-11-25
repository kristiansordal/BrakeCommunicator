#include "fast_matrix_market/app/triplet.hpp"
#include "matrix.hpp"
#include <chrono>
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

    auto ttots = chrono::high_resolution_clock::now();
    double tfile;
    double tcomp;
    double tcomp_kernel;

    vector<i64> rc;
    vector<ulli> send_counts;

    auto t1 = chrono::high_resolution_clock::now();
    read_file(argc[1], M);
    auto t2 = chrono::high_resolution_clock::now();
    tfile = chrono::duration<double>(t2 - t1).count();
    M.init_row_ptr();
    M.init_row_size();
    M.init_v_old();
    M.init_v_new();

    t1 = chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; i++) {
        M.update();
    }
    t2 = chrono::high_resolution_clock::now();
    tcomp = chrono::duration<double>(t2 - t1).count();

    double sum = 0;
    ulli ops = (ulli)M.nnz * 2 * 100;

    for (auto &i : M.v_old) {
        sum += abs(i * i);
    }
    auto ttote = chrono::high_resolution_clock::now();

    cout << endl;
    cout << "File:             " << tfile << endl;
    cout << "Total:            " << chrono::duration<double>(ttote - ttots).count() << endl;
    cout << "L2 norm:          " << sqrt(sum) << endl;
    cout << "OPS:              " << ops << endl;
    cout << "GFLOPS:           " << ops / (tcomp * 1e9) << endl;

    return 0;
}
