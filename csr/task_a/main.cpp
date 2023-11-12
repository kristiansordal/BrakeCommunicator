#include "fast_matrix_market/app/triplet.hpp"
#include "matrix.hpp"
#include <bits/stdc++.h>
#include <boost/mpi.hpp>
#include <fast_matrix_market/fast_matrix_market.hpp>
#include <metis.h>
#include <optional>

using namespace std;
namespace mpi = boost::mpi;
namespace ffm = fast_matrix_market;

// TODO: Implement hybrid version (OpenMP + MPI version) of Compressed Sparse Row sparse
// TODO: Matrix-vector multiplication using  1d or 1e strategy with file  reading.
// TODO: Implement file reading.
// TODO: Use metis for load balancing (PartKwayGraph)
// TODO: Check out https://sparcityeu.github.io/sparsebase/pages/getting_started/ for file reading
//

void read_file(string path, Matrix &matrix) {
    ifstream file;

    file.open(path);

    if (!file.is_open()) {
        cerr << "Error: Could not open file." << endl;
        return;
    }

    ffm::read_matrix_market_triplet(file, matrix.nrows, matrix.ncols, matrix.row_ptr, matrix.col_ptr, matrix.a);
    cout << matrix.nnz << endl;

    cout << "Successfully read file!" << endl;
    file.close();
}

int main() {

    mpi::environment env;
    mpi::communicator world;
    mpi::timer time;
    Matrix A;

    if (world.rank() == 0) {
        read_file("494_bus/494_bus.mtx", A);

        cout << A.nrows << endl;
    }

    return 0;
}
