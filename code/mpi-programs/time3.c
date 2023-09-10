#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
    int myrank;
    int np;
    int x;
    MPI_Status status;
    int i;
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

    // if (myrank == 0) {
    //     printf("Give data size to send: ");
    //     scanf("%d", &ss);
    // }

    for (int ss = 0; ss < 1000; ss++) {
        MPI_Bcast(&ss, 1, MPI_INT, 0, MPI_COMM_WORLD);
        data = (int *)malloc(sizeof(int) * ss * 1000);

        if (myrank == 0) {
            start = MPI_Wtime();

            MPI_Send(data, ss, MPI_INT, 1, 1, MPI_COMM_WORLD);
            MPI_Recv(data, ss, MPI_INT, 1, 1, MPI_COMM_WORLD, &status);

            end = MPI_Wtime();

            printf("Data size is: %d\n", *data);
            printf("Total time is %f\n\n", end - start);
        } else {
            MPI_Recv(data, ss, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Send(data, ss, MPI_INT, 0, 1, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}
