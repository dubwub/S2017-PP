/***************************************************************************/
/* Template for Asssignment 3 **********************************************/
/* Darwin Ding             **********************************************/
/***************************************************************************/

/***************************************************************************/
/* Includes ****************************************************************/
/***************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>
// Local RNG Include for Assignment 3
#include"clcg4.h"
#include<mpi.h>
#include <time.h>


/***************************************************************************/
/* Defines *****************************************************************/
/***************************************************************************/

#define ALIVE 1
#define DEAD  0

#define WIDTH 8192
#define HEIGHT 8192
#define TICKS 100

/***************************************************************************/
/* Global Vars *************************************************************/
/***************************************************************************/

int** pretickSlice; // to be init in main function
int** posttickSlice; // to be init with each tick

/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/

// You define these


/***************************************************************************/
/* Function: Main **********************************************************/
/***************************************************************************/

int main(int argc, char *argv[])
{
  clock_t start, end;
  MPI_Status status;
  int mpi_myrank;
  int mpi_commsize;

  start = clock();
  
  // Example MPI startup and using CLCG4 RNG
  MPI_Init( &argc, &argv);
  MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize);

  if (argc < 2) {
    printf("usage: mpirun -np 4 ./program threshold\n");
    exit(1);
  }
  float threshold = atof(argv[1]);
  
  MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank);

  // create board + 2 ghost rows
  pretickSlice = malloc(((HEIGHT / mpi_commsize) + 2) * sizeof(int*));
  for (int i = 0; i < (HEIGHT / mpi_commsize) + 2; i++) {
    pretickSlice[i] = malloc(WIDTH * sizeof(int));
  }

  // create post tick board without ghost rows
  posttickSlice = malloc((HEIGHT / mpi_commsize) * sizeof(int*));
  for (int i = 0; i < (HEIGHT / mpi_commsize); i++) {
    posttickSlice[i] = malloc(WIDTH * sizeof(int));
  }
    
  // Init 16,384 RNG streams - each rank has an independent stream
  InitDefault();
    
  // Note, used the mpi_myrank to select which RNG stream to use.
  // You must replace mpi_myrank with the right row being used.
  // This just show you how to call the RNG.    
  // printf("Rank %d of %d has been started and a first Random Value of %lf\n", 
  // mpi_myrank, mpi_commsize, GenVal(mpi_myrank));

  // init this part of the board slice (avoid writing in ghost rows)
  for (int i = 0; i < (HEIGHT/mpi_commsize) + 2; i++) {
    for (int j = 0; j < WIDTH; j++) {
      if (i == 0 || i == (HEIGHT/mpi_commsize) + 1) {
	pretickSlice[i][j] = DEAD;
	continue;
      }
      if (GenVal(i) > 0.5) {
	pretickSlice[i][j] = ALIVE;
      } else {
	pretickSlice[i][j] = DEAD;
      }
    }
  }

  for (int i = 0; i < (HEIGHT/mpi_commsize); i++) {
    for (int j = 0; j < WIDTH; j++) {
      posttickSlice[i][j] = DEAD;
    }
  }

  MPI_Barrier( MPI_COMM_WORLD );
    
  // Insert your code
  for (int tick = 0; tick < TICKS; tick++) {
    // first step, all evens send their tops to the previous rank
    // then, all odds, send their tops to the previous rank
    // then all evens send their bottoms to next rank
    // then all odds send their bottoms to next rank
    if (mpi_commsize > 1) { // avoid dumb edge cases where commsize = 1 so we don't need to actually pass data
      if (mpi_myrank % 2 == 0) {
	if (mpi_myrank > 0) {
	  MPI_Send(&pretickSlice[1][0], WIDTH, MPI_INT, mpi_myrank - 1, 1, MPI_COMM_WORLD);
	}
	MPI_Recv(&pretickSlice[(HEIGHT/mpi_commsize) + 1][0], WIDTH, MPI_INT, mpi_myrank + 1, 2, MPI_COMM_WORLD, &status);
	MPI_Send(&pretickSlice[(HEIGHT/mpi_commsize)][0], WIDTH, MPI_INT, mpi_myrank + 1, 3, MPI_COMM_WORLD);
	if (mpi_myrank > 0) {
	  MPI_Recv(&pretickSlice[0][0], WIDTH, MPI_INT, mpi_myrank - 1, 4, MPI_COMM_WORLD, &status);
	}
      } else {
	if (mpi_myrank < mpi_commsize - 1) {
	  MPI_Recv(&pretickSlice[(HEIGHT/mpi_commsize) + 1][0], WIDTH, MPI_INT, mpi_myrank + 1, 1, MPI_COMM_WORLD, &status);
	}
	MPI_Send(&pretickSlice[1][0], WIDTH, MPI_INT, mpi_myrank - 1, 2, MPI_COMM_WORLD);
	MPI_Recv(&pretickSlice[0][0], WIDTH, MPI_INT, mpi_myrank - 1, 3, MPI_COMM_WORLD, &status);
	if (mpi_myrank < mpi_commsize - 1) {
	  MPI_Send(&pretickSlice[(HEIGHT/mpi_commsize)][0], WIDTH, MPI_INT, mpi_myrank + 1, 4, MPI_COMM_WORLD);
	}
      }
    }

    // second step, each pretick calculation goes into posttick, and then we memcpy back into pretick for the next step
    for (int i = 0; i < (HEIGHT/mpi_commsize); i++) {
      for (int j = 0; j < WIDTH; j++) {
	if (GenVal(i) < threshold) {
	  if (GenVal(i) > 0.5) {
	    posttickSlice[i][j] = ALIVE;
	  } else {
	    posttickSlice[i][j] = DEAD;
	  }
	  continue;
	}

	int num_alive_neighbors = 0;
	if (j - 1 >= 0) {
	  if (pretickSlice[i][j-1] == ALIVE) num_alive_neighbors++;
	  if (pretickSlice[i+1][j-1] == ALIVE) num_alive_neighbors++;
	  if (pretickSlice[i+2][j-1] == ALIVE) num_alive_neighbors++;
	}
	if (j + 1 < WIDTH) {
	  if (pretickSlice[i][j+1] == ALIVE) num_alive_neighbors++;
	  if (pretickSlice[i+1][j+1] == ALIVE) num_alive_neighbors++;
	  if (pretickSlice[i+2][j+1] == ALIVE) num_alive_neighbors++;
	}
	if (pretickSlice[i][j] == ALIVE) num_alive_neighbors++;
	if (pretickSlice[i+2][j] == ALIVE) num_alive_neighbors++;

	
	if (num_alive_neighbors < 2 && pretickSlice[i+1][j] == ALIVE) { // underpopulation
	  posttickSlice[i][j] = DEAD;
	}
	else if ((num_alive_neighbors == 2 || num_alive_neighbors == 3) && pretickSlice[i+1][j] == ALIVE) { // lives on
	  posttickSlice[i][j] = ALIVE;
	}
	else if (num_alive_neighbors > 3 && pretickSlice[i+1][j] == ALIVE) { // overpopulation
	  posttickSlice[i][j] = DEAD;
	}
	else if (num_alive_neighbors == 3 && pretickSlice[i+1][j] == DEAD) { // reproduction
	  posttickSlice[i][j] = ALIVE;
	}
	else { // they ded
	  posttickSlice[i][j] = DEAD;
	}
      }
    }
  }

  // END -Perform a barrier and then leave MPI
  MPI_Barrier( MPI_COMM_WORLD );

  /* for (int i = 0; i < (HEIGHT / mpi_commsize) + 2; i++) { // cleanup
    free(pretickSlice[i]);
  }
  free(pretickSlice);

  for (int i = 0; i < (HEIGHT / mpi_commsize); i++) { // cleanup
    free(posttickSlice[i]);
  }
  free(posttickSlice); */

  end = clock();
  if (mpi_myrank == 0) {
    printf("%f s elapsed\n", (double) (end - start)/CLOCKS_PER_SEC);
  }
  MPI_Finalize();
  return 0;
}

/***************************************************************************/
/* Other Functions - You write as part of the assignment********************/
/***************************************************************************/
