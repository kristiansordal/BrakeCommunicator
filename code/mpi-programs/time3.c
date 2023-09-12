#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int myrank;
    int np;
    MPI_Status status;
    double start, end;
    int *data;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if (np != 2) {
        printf("This program can only be run with 2 processors\n");
        MPI_Finalize();
        return 0;
    }

    int size = 2000000;

    data = (int *)malloc(sizeof(int) * size);

    if (myrank == 0) {
        for (int i = 1; i < size; i *= 2) {
            start = MPI_Wtime();
            MPI_Send(data, i, MPI_INT, 1, 0, MPI_COMM_WORLD);
            end = MPI_Wtime();

            printf("Message size: %d\nTime taken: %f seconds\n\n", i, (end - start));
        }
    } else {
        for (int i = 1; i < size; i *= 2) {
            MPI_Recv(data, i, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        }
    }

    MPI_Finalize();
    return 0;
}
