#include <stdio.h>
#include <mpi.h>

/* This program compares the time for two broadcast operations
 * with the time for processor 0 sending to everyone and everyone
 * sending back to processor 0.
 */

#define size 1000000
int main(argc,argv) 
int argc;
char **argv;
{

  int myrank;
  int np;
  int x;
  MPI_Status status;
  int i,ss;
  double start,end;
  int data[size];


  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  
  ss = 10000;

// Everybody must know the number of elements being sent
  MPI_Bcast(&ss,1,MPI_INT,0,MPI_COMM_WORLD);

  MPI_Barrier(MPI_COMM_WORLD);  // Wait for everyone

  start = MPI_Wtime();
  MPI_Bcast(&data,ss,MPI_INT,0,MPI_COMM_WORLD); // Send from 0 to everyone
  MPI_Bcast(&data,ss,MPI_INT,1,MPI_COMM_WORLD); // Send from 1 to everyone
  end = MPI_Wtime();

  if (myrank == 0) {
    printf("Total time for bcast is %f \n",end-start);
  }

// Why did we do two broadcasts?


  MPI_Barrier(MPI_COMM_WORLD);

  if (myrank == 0) {
    start = MPI_Wtime();

// Send to every processor
    for(i=1;i<np;i++) {
      MPI_Send(data,ss,MPI_INT,i,1,MPI_COMM_WORLD);
    }
// Receive from processor 1
    MPI_Recv(data,ss,MPI_INT,1,2,MPI_COMM_WORLD,&status);

    end = MPI_Wtime();

    printf("Total time for my bcast is %f \n",end-start);
  }

  else {
    MPI_Recv(data,ss,MPI_INT,0,1,MPI_COMM_WORLD,&status);

    if (myrank == 1) {
      MPI_Send(data,ss,MPI_INT,i,2,MPI_COMM_WORLD); // Send to proc 0
      for(i=2;i<np;i++)     			// Send to every other proc
        MPI_Send(data,ss,MPI_INT,i,2,MPI_COMM_WORLD);
    }
    else  // Receive from 1
      MPI_Recv(data,ss,MPI_INT,1,2,MPI_COMM_WORLD,&status);

  }

  MPI_Finalize();
  return(0);
}
