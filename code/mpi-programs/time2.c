#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/* This program takes the time for sending a number of integers
 * from processor 0 to processor 1 and back again
 */

#define iterations 100000

int main(argc,argv) 
int argc;
char **argv;
{

  int myrank;
  int np;
  MPI_Status status;
  int i;
  double start,end;
  int data;


  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  
  if (np != 2) {
    printf("This program can only be run with 2 processors\n");
    MPI_Finalize();
    return(0);
  }

  if (myrank == 0) {
    start = MPI_Wtime();   // Note only proc 0 is taking time

    for(i=0;i<iterations;i++) {
      MPI_Send(&data,1,MPI_INT,1,1,MPI_COMM_WORLD);         // Send to proc 1
      MPI_Recv(&data,1,MPI_INT,1,1,MPI_COMM_WORLD,&status); // Recv from proc 1
    }

    end = MPI_Wtime();

    printf("Total time is %f \n",end-start);
    printf("Time per sending is %10.10f \n",(end-start)/(2*iterations));
   
  }
  else {  // This is what proc 1 does
    for(i=0;i<iterations;i++) {
      MPI_Recv(&data,1,MPI_INT,0,1,MPI_COMM_WORLD,&status);
      MPI_Send(&data,1,MPI_INT,0,1,MPI_COMM_WORLD);
    }
  }

  // Why is data sent back and forth?

  MPI_Finalize();
  return(0);
}
