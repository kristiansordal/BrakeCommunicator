#include <stdio.h>
#include <mpi.h>

/* This program shows how to use isend and ireceive
 */

int main(argc,argv) 
int argc;
char **argv;
{

  int myrank;
  int np;
  int x;
  MPI_Status status;
  MPI_Request req;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  
  printf("I am %d out of %d processors \n",myrank,np);

  if (myrank == 0) {
    x = 11;
    MPI_Isend(&x,1,MPI_INT,1,1,MPI_COMM_WORLD,&req);
    MPI_Wait(&req,&status);
  }
  else {
    MPI_Irecv(&x,1,MPI_INT,0,1,MPI_COMM_WORLD,&req);
    MPI_Wait(&req,&status);
    printf("Processor %d got %d \n",myrank,x);
  }

  MPI_Finalize();
  return(0);
}
