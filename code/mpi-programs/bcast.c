#include <stdio.h>
#include <mpi.h>

/* This program shows a simple example of using broadcast
 * from processor 0
 */

int main(argc,argv) 
int argc;
char **argv;
{

  int myrank;
  int np;
  int x;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  
  printf("I am %d out of %d processors \n",myrank,np);

  if (myrank == 0) {
    x = 11;
  }

  MPI_Bcast(&x,1,MPI_INT,0,MPI_COMM_WORLD);

  printf("Processor %d got %d \n",myrank,x);

  MPI_Finalize();
  return(0);
}
