#include <malloc/_malloc.h>
#include <mpi.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);               /* starts MPI */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); /* get current process id */
    MPI_Comm_size(MPI_COMM_WORLD, &size); /* get number of processes */

    size_t total_length = 0;
    for (int i = 1; i < argc; i++) {
        total_length += strlen(argv[i]) + 1; // +1 for space or null terminator
    }

    // Allocate memory for the concatenated string
    char *args = (char *)malloc(total_length);
    if (args == NULL) {
        perror("Memory allocation failed");
        return 1;
    }

    // Initialize the concatenated string
    args[0] = '\0';

    // Concatenate the arguments into the string
    for (int i = 1; i < argc; i++) {
        strcat(args, argv[i]);
        if (i < argc - 1) {
            strcat(args, " "); // Add a space between arguments
        }
    }

    // printf("Concatenated string: %s\n", args);

    printf("Hello world from process %d of %d. This is argc: %d, this is argv: %d\n", rank, size, argc, *args);
    MPI_Finalize();
    return 0;
}
