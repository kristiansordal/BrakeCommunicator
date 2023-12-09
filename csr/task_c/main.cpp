#include "fast_matrix_market/app/triplet.hpp"
#include "matrix.hpp"
#include <boost/mpi.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <fast_matrix_market/fast_matrix_market.hpp>
#include <fstream>

using namespace std;
namespace ffm = fast_matrix_market;
int main(int argc, char **argv) {

    Matrix M;

    MPI_Init(&argc, &argv);

    int rank, np;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    vector<i64> row_ptr;
    vector<i64> row_ids;
    vector<i64> col_ptr;
    vector<double> vals;
    vector<i64> rc;
    int nrows, ncols;

    rc.assign(np, 0);

    // Read file on all ranks
    ifstream file;
    file.open(argv[1]);

    cout << "Opening file..." << endl;

    if (!file.is_open()) {
        cout << "Error: Could not open file." << endl;
        return 0;
    }

    cout << "File successfully opened" << endl;

    ffm::read_matrix_market_triplet(file, nrows, ncols, col_ptr, row_ids, vals);

    cout << "Successfully read file" << endl;

    int nnz = vals.size();
    int n = nrows;

    file.close();
    // Done reading files

    // Initialize row pointer
    vector<i64> r;
    int count = 1;

    r.push_back(0);
    for (int i = 1; i < row_ids.size(); i++) {
        if (row_ids[i] == row_ids[i - 1]) {
            count++;
        } else {
            r.push_back(count++);
        }
    }

    r.push_back(count);
    row_ptr = r;
    // Done initializing row ptr

    MPI_Barrier(MPI_COMM_WORLD);

    // Start load balancing
    if (rank == 0) {
        int avg_load = nnz / np;
        int core = 0;

        for (int i = 0; i < nrows; i++) {
            rc[core]++;

            if (row_ptr[i + 1] > avg_load * (core + 1)) {
                core++;
            }
        }
        for (auto &i : rc) {
            cout << i << " ";
        }
        cout << endl;
    }

    MPI_Bcast(rc.data(), np * sizeof(int), MPI_INT, 0, MPI_COMM_WORLD);

    for (auto &i : rc) {
        cout << i << " ";
    }
    cout << endl;
    // End load balancing

    return 0;
}
