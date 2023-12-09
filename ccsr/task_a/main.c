#include "matrix.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void read_file(struct Matrix *M, char **argv) {
    const char *csr_file_name = argv[1];
    FILE *file_in = fopen(csr_file_name, "rb");

    if (!file_in) {
        perror("Error opening file");
    }

    // Read matrix data

    if (fread(&M->n, sizeof(M->n), 1, file_in) != 1 || M->n <= 0 ||
        fread(&M->ncols, sizeof(M->ncols), 1, file_in) != 1 || M->ncols <= 0 ||
        fread(&M->nnz, sizeof(M->nnz), 1, file_in) != 1 || M->nnz <= 0) {
        fprintf(stderr, "Error reading CSR header section from binary file %s\n", csr_file_name);
        fclose(file_in);
    }

    M->nrows = M->n;
    M->vals = (double *)malloc(sizeof(double) * M->nnz);
    M->row_ptr = (int *)malloc(sizeof(int) * (M->n + 1));
    M->col_ptr = (int *)malloc(sizeof(int) * M->nnz);

    if (fread(M->row_ptr, sizeof(int), M->n + 1, file_in) != M->n + 1 ||
        fread(M->col_ptr, sizeof(int), M->nnz, file_in) != M->nnz ||
        fread(M->vals, sizeof(double), M->nnz, file_in) != M->nnz) {
        fprintf(stderr, "Error reading matrix data from binary file %s\n", csr_file_name);
        free(M->vals);
        free(M->row_ptr);
        free(M->col_ptr);
        fclose(file_in);
    }

    fclose(file_in);
}

int main(int argc, char **argv) {
    int rank;
    int np;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    struct Matrix M;
    int *rc = (int *)malloc(sizeof(int) * np);

    if (rank == 0) {
        if (argc != 2) {
            fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
            return 1;
        }

        // reads the file
        read_file(&M, argv);

        // perform Load balancing
        double avg_load = (double)M.nnz / (double)np;
        int c = 0;

        for (int i = 0; i < M.nrows; i++) {
            rc[c]++;

            if (M.row_ptr[i + 1] > avg_load * (c + 1)) {
                c++;
            }
        }

        M.nrows = rc[rank];

        // send the amount of rows each rank should have
        for (int i = 1; i < np; i++) {
            MPI_Send(&rc[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

    } else {
        MPI_Recv(&M.nrows, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Should determine send lists

    // All ranks gets the amount of
    MPI_Bcast(&M.n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    init_v_old(&M);

    int **rb = (int **)malloc(sizeof(int *) * np);
    int **cb = (int **)malloc(sizeof(int *) * np);
    double **vb = (double **)malloc(sizeof(double *) * np);

    // Store the last row_ptr of the ranks
    int *sizes = (int *)malloc(sizeof(int) * np);

    if (rank == 0) {
        int offset = 0;

        for (int i = 0; i < np; i++) {
            rb[i] = (int *)malloc(sizeof(int) * (rc[i] + 1));
            for (int j = offset; j < rc[i] + 1 + offset; j++) {
                rb[i][j] = M.row_ptr[j];
                sizes[i]++;
            }
            offset += rc[i];
        }

        rb[np - 1] = (int *)malloc(sizeof(int) * (rc[np - 1] + 1));
        rb[np - 1][rc[np - 1]] = M.row_ptr[M.nrows];

        for (int i = 0; i < np; i++) {
            for (int j = i == 0 ? 0 : rb[i - 1][sizes[i - 1] - 1]; j < rb[i][sizes[i] - 1]; j++) {
                cb[i][j] = M.col_ptr[j];
                vb[i][j] = M.vals[j];
            }
        }

        M.row_ptr = rb[0];
        M.col_ptr = cb[0];
        M.vals = vb[0];
    }
    return 0;
}
