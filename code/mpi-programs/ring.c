#include <mpi.h>
#include <stdio.h>

/* This program shows communication starting with processor 0
 * which sends to processor 1 and so on, until the message reaches
 * processor 0 again.
 */

int main(int argc, char **argv) {

    int myrank;
    int np;
    int x;
    MPI_Status status;
    MPI_Request req;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    printf("I am %d out of %d processors \n", myrank, np);

    if (myrank == 0) {
        x = 11;
        MPI_Send(&myrank, 1, MPI_INT, (myrank - 1 + np) % np, 1, MPI_COMM_WORLD);
        MPI_Recv(&myrank, 1, MPI_INT, (myrank + 1) % np, 1, MPI_COMM_WORLD, &status);
        printf("Processor %d got %d from %d \n", myrank, myrank, (myrank - 1 + np) % np);
    } else {
        MPI_Recv(&x, 1, MPI_INT, (myrank + 1) % np, 1, MPI_COMM_WORLD, &status);
        printf("Processor %d got %d from %d \n", myrank, x, (myrank + 1) % np);
        MPI_Send(&x, 1, MPI_INT, (myrank - 1 + np) % np, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return (0);
}
