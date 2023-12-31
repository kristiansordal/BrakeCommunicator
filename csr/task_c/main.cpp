#include "fast_matrix_market/app/triplet.hpp"
#include "matrix.hpp"
#include <fast_matrix_market/fast_matrix_market.hpp>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <numeric>

using i64 = long long int;
using namespace std;
namespace ffm = fast_matrix_market;
int main(int argc, char **argv) {

    int rank, np;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    // Timer variables
    double ttstart, ttend, tcstart, tcend, tcommstart, tcommend, tfilestart, tfileend;

    // The matrices
    vector<i64> row_ptr;
    vector<i64> row_ids;
    vector<i64> col_ptr;
    vector<double> vals;
    vector<i64> rc;
    int nrows, ncols;
    rc.assign(np, 0);

    // Read file on all ranks
    tfilestart = MPI_Wtime();
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
    tfileend = MPI_Wtime();
    ttstart = MPI_Wtime();

    int nnz = vals.size();
    int n = nrows;

    file.close();
    // Done reading files

    // Initialize row pointer
    vector<i64> r;
    int count = 1;

    cout << "Init row ptr" << endl;
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
    cout << "Done init row ptr" << endl;
    // Done initializing row ptr

    MPI_Barrier(MPI_COMM_WORLD);

    cout << "Load Balancing start" << endl;
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
    }

    cout << "Broacasting load balance" << endl;
    MPI_Bcast(rc.data(), np * sizeof(int), MPI_INT, 0, MPI_COMM_WORLD);
    cout << "End load balance" << endl;

    nrows = rc[rank];
    vector<double> v_old(n, 1);
    vector<double> v_new(nrows, 0);

    // Get displacements and recvcounts for all gather later
    int offset = accumulate(rc.begin(), rc.begin() + rank, 0);
    vector<int> displs(np, 0);
    MPI_Allgather(&offset, 1, MPI_INT, displs.data(), 1, MPI_INT, MPI_COMM_WORLD);

    vector<int> recvcounts(np, 0);
    int v = v_new.size();
    MPI_Allgather(&v, 1, MPI_INT, recvcounts.data(), 1, MPI_INT, MPI_COMM_WORLD);

    // resize to fit their rank size
    col_ptr.assign(col_ptr.begin() + row_ptr[offset], col_ptr.begin() + row_ptr[offset + nrows]);
    vals.assign(vals.begin() + row_ptr[offset], vals.begin() + row_ptr[offset + nrows]);
    row_ptr.assign(row_ptr.begin() + offset, row_ptr.begin() + offset + nrows);

    ncols = col_ptr.size();

    // Finally start the computation
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "Start computation" << endl;
    }
    // Perform 100 multiplications
    for (int r = 0; r < 100; r++) {
        if (rank == 0) {
            cout << r << "%" << endl;
        }
        int start = row_ptr[0];

        // Use OpenMP to parallelize the for loop
        for (int row = 0; row < nrows; row++) {
            double sum = 0.0;

            // We sum up the  values in each row
            for (int i = row_ptr[row]; i < row_ptr[row + 1]; i++) {
                sum += vals[i] + v_old[col_ptr[i]];
            }

            v_new[row] = sum;
        }

        v_old.clear();
        v_old.assign(n, 0);

        /*
         * v_new.data()      -> The newly computed data
         * v                 -> The size of the data to be sent
         * MPI_DOUBLE        -> MPI Data type of the contents being sent
         * v_old.data()      -> The buffer to receive the data
         * recvcounts.data() -> The size of the data to be received
         * displs.data()     -> The displacement of the data to be received
         * MPI_COMM_WORLD    -> The communicator
         *
         * */

        if (rank == 0) {
            cout << "Start allgather" << endl;
        }

        MPI_Allgatherv(v_new.data(), v, MPI_DOUBLE, v_old.data(), recvcounts.data(), displs.data(), MPI_DOUBLE,
                       MPI_COMM_WORLD);

        if (rank == 0) {
            cout << "End allgather" << endl;
        }
    }
    ttend = MPI_Wtime();

    if (rank == 0) {
        unsigned long long int ops = 2ll * (unsigned long long int)nnz * 100ll;
        cout << "Total time: " << ttend - ttstart << endl;
        cout << "Total file: " << tfileend - tfilestart << endl;
        cout << "Operations: " << ops << endl;
        cout << "GFLOPS:     " << ops / ((ttend - ttstart) * 1e9) << endl;
    }
    MPI_Finalize();

    return 0;
}
