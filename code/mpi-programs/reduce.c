#include <stdio.h>
#include <mpi.h>

/* This program shows the use of the reduce operation
 */

int main(argc,argv) 
int argc;
char **argv;
{

  int myrank;
  int np;
  int x1,x2;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  
/*
  printf("I am %d out of %d processors \n",myrank,np);
*/


  MPI_Reduce(&myrank,&x1,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
  MPI_Reduce(&myrank,&x2,1,MPI_INT,MPI_MAX,0,MPI_COMM_WORLD);

  if (myrank == 0) {
    printf("Sum on processor %d: %d, should be %d \n",myrank,x1,(np-1)*np/2);
    printf("Max on processor %d: %d, should be %d \n",myrank,x2,np-1);
  }

  MPI_Finalize();
  return(0);
}
