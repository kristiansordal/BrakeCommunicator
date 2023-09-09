#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/* This program takes the time for sending a number of integers
 * from processor 0 to processor 1 and back again
 */

#define size 1000
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
  int *data;


  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  
  if (np != 2) {
    printf("This program can only be run with 2 processors\n");
    MPI_Finalize();
    return(0);
  }

  if (myrank == 0) {
    printf("Give data size to send \n");
    scanf("%d",&ss);
  }


  MPI_Bcast(&ss,1,MPI_INT,0,MPI_COMM_WORLD);

  data = (int *) malloc(sizeof(int)*ss);  // allocate space

  if (myrank == 0) {
    start = MPI_Wtime();   // Note only proc 0 is taking time

    MPI_Send(data,ss,MPI_INT,1,1,MPI_COMM_WORLD);         // Send to proc 1
    MPI_Recv(data,ss,MPI_INT,1,1,MPI_COMM_WORLD,&status); // Recv from proc 1

    end = MPI_Wtime();

    printf("Total time is %f \n",end-start);
    printf("Time per byte is %10.10f \n",(end-start)/(ss*4*2));
   
  }
  else {  // This is what proc 1 does
    MPI_Recv(data,ss,MPI_INT,0,1,MPI_COMM_WORLD,&status);
    MPI_Send(data,ss,MPI_INT,0,1,MPI_COMM_WORLD);
  }

  // Why is data sent back and forth?

  MPI_Finalize();
  return(0);
}
