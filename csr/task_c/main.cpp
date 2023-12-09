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
    vector<i64> col_ptr;
    vector<double> vals;
    int nrows, ncols;

    ifstream file;
    file.open(argv[1]);

    cout << "Opening file..." << endl;

    if (!file.is_open()) {
        cout << "Error: Could not open file." << endl;
        return 0;
    }

    cout << "File successfully opened" << endl;

    ffm::read_matrix_market_triplet(file, nrows, ncols, col_ptr, row_ptr, vals);

    cout << "Successfully read file" << endl;

    int nnz = vals.size();
    int n = nrows;

    file.close();

    if (rank == 0) {
        cout << "Sending" << endl;
        for (int i = 0; i < np; i++) {
            if (i != rank) {
                MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
    } else {
        n = 0;
        MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << rank << " has  " << n << endl;
    }
    // vector<i64> r;
    // int count = 1;

    // r.push_back(0);
    // for (int i = 1; i < row_ptr.size(); i++) {
    //     if (row_ptr[i] == row_ptr[i - 1]) {
    //         count++;
    //     } else {
    //         r.push_back(count++);
    //     }
    // }
    // r.push_back(count);
    // row_ptr = r;

    return 0;
}
