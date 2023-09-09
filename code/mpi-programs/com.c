#include <mpi.h>
#include <stdio.h>
#include <unistd.h>

/* Program showing communication from processor 0 to every other processor
 */

int main(int argc, char **argv) {

    int myrank;
    int np;
    int x;
    MPI_Status status;
    int i;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    printf("I am %d out of %d processors \n", myrank, np);

    if (myrank == 0) {
        sleep(2);

        int y = 0;
        for (i = 1; i < np; i++) {
            // Send the number i to process i
            MPI_Send(&i, 1, MPI_INT, i, 1, MPI_COMM_WORLD);

            // Recieve back the number multiplied
            MPI_Recv(&x, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);

            // Accumulate
            y += x;
        }

        // Print it
        printf("y: %d", y);
    } else {

        // Recieves the number i sent from process 0
        MPI_Recv(&x, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

        // Multiplies it and sends it on
        x *= 2;
        MPI_Send(&x, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return (0);
}
