 /*
  "Hello World" MPI Test Program
 */
#include <mpi.h>
#include <stdio.h>
#include <string.h>
 
#define BUFSIZE 128
#define TAG 0
#define RANDOM_ARRAY_SIZE 1000
#define RANDOM_ARRAY_TAG 1
 
int main(int argc, char *argv[]) {
  char idstr[32];
  char buff[BUFSIZE];
  int numprocs;
  int myid;
  int i;
  MPI_Status stat;
  /* MPI programs start with MPI_Init; all 'N' processes exist thereafter */
  MPI_Init(&argc,&argv);
  /* find out how big the SPMD world is */
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  /* and this processes' rank is */
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);
 
  /* At this point, all programs are running equivalently, the rank
     distinguishes the roles of the programs in the SPMD model, with
     rank 0 often used specially... */
  if(myid == 0) {
    printf("%d: We have %d processors\n", myid, numprocs);
 
    for(i=1;i<numprocs;i++) {
      sprintf(buff, "Hello %d! ", i);
      MPI_Send(buff, BUFSIZE, MPI_CHAR, i, TAG, MPI_COMM_WORLD);
    }
    for(i=1;i<numprocs;i++) {
      MPI_Recv(buff, BUFSIZE, MPI_CHAR, i, TAG, MPI_COMM_WORLD, &stat);
      printf("%d: %s\n", myid, buff);
    }

    getchar();
    double randomArray[RANDOM_ARRAY_SIZE] = {};
    for (i = 0; i < RANDOM_ARRAY_SIZE; ++i) {
      randomArray[i] = (double) i;
    }

    int dataSize = RANDOM_ARRAY_SIZE/(numprocs - 1);
    double *startPoint = randomArray;
    int i;
    
    for (i = 1; i < numprocs; i++) {
      if (i == numprocs - 1) {
	dataSize = RANDOM_ARRAY_SIZE - (i - 1)*dataSize;
      }
      MPI_Send(startPoint, dataSize, MPI_DOUBLE, i, 
	       RANDOM_ARRAY_TAG, MPI_COMM_WORLD);
      startPoint += dataSize;
    }
    
    double tempRandomElementsSum = 0.0;
    double randomSum = 0.0;

    for (i = 1; i < numprocs; i++) {
      MPI_Recv(&tempRandomElementsSum, 1, MPI_DOUBLE, i, 
	       RANDOM_ARRAY_TAG, MPI_COMM_WORLD, &stat);
      printf("Get Sum %e from slave %d\n", 
	     tempRandomElementsSum, i);
      randomSum += tempRandomElementsSum;
    }
    printf("Random Sum %e, expected is %e\n", 
	   randomSum, 
	   ((double) RANDOM_ARRAY_SIZE - 1 + 0)/2*
	   RANDOM_ARRAY_SIZE);

  } else {
    /* receive from master (rank = 0): */
    MPI_Recv(buff, BUFSIZE, MPI_CHAR, 0, TAG, MPI_COMM_WORLD, &stat);
    sprintf(idstr, "Processor %d ", myid);
    strncat(buff, idstr, BUFSIZE-1);
    strncat(buff, "reporting for duty", BUFSIZE-1);
    /* send to master (rank = 0): */
    MPI_Send(buff, BUFSIZE, MPI_CHAR, 0, TAG, MPI_COMM_WORLD);

    /* calculate sum of elements gotten from master (rank = 0) */
    int dataSize = RANDOM_ARRAY_SIZE/(numprocs - 1);
    if (myid == numprocs) {
      dataSize = RANDOM_ARRAY_SIZE - (numprocs - 2)*dataSize;
    }
    double randomArray[RANDOM_ARRAY_SIZE] = {};
    /* get master data from master (rank = 0)*/
    MPI_Recv(randomArray, dataSize, MPI_DOUBLE, 0, 
	     RANDOM_ARRAY_TAG, MPI_COMM_WORLD, &stat);

    /* calculate sum */
    double randomSum = 0.0;
    for (i = 0; i < dataSize; i++) {
      randomSum += randomArray[i];
    }
      
    /* send sum to master (rank = 0)*/
    MPI_Send(&randomSum, 1, MPI_DOUBLE, 0, 
	     RANDOM_ARRAY_TAG, MPI_COMM_WORLD);
  }
 
  /* MPI programs end with MPI Finalize; 
     this is a weak synchronization point. */
  MPI_Finalize();
  return 0;
}
