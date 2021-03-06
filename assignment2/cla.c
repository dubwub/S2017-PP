/*
  Parallel MPI CLA Adder by Darwin Ding
  Usage: mpicc cla.c -o cla -Wall (no warnings!)
  mpirun -np <num_processes either 1,2,4,8> cla > output

  To verify the large blob output (without splitting), edit REALOUTPUT_FILENAME and uncomment verifyResult call in main
  To enable/disable MPI_Barrier, toggle EnableBarrier #define below
 */

#include "cla.h"
#include <mpi.h>
#include <time.h>

#define input_size 524288
#define block_size 8

#define EnableBarrier 0 // MPI_Barrier toggle

#define digits (input_size)
#define bits digits*4
#define ngroups bits/block_size
#define s1 ngroups/block_size
#define s2 s1/block_size
#define s3 s2/block_size
#define s4 s3/block_size
// s5 == block_size

#define REALOUTPUT_FILENAME "resultfile2"

//Global definitions of the various arrays used in steps for easy access
int gi[bits] = {0};
int pi[bits] = {0};

int ggj[ngroups] = {0};
int gpj[ngroups] = {0};

int sgk[s1] = {0};
int spk[s1] = {0};
int s2gl[s2] = {0};
int s2pl[s2] = {0};
int s3gm[s3] = {0};
int s3pm[s3] = {0};
int s4gn[s4] = {0};
int s4pn[s4] = {0};
int s5go[block_size] = {0};
int s5po[block_size] = {0};

int s5co[block_size] = {0};
int s4cn[s4] = {0};
int s3cm[s3] = {0};
int s2cl[s2] = {0};
int sck[s1] = {0};

int gcj[ngroups] = {0};

int ci[bits] = {0};

int sumi[bits] = {0};

//Integer array of inputs in binary form
int* bin1;
int* bin2;

//Character array of inputs in hex form
char* hex1;
char* hex2;

void read_input(int pid, int num_processes){
  FILE* f;
  int i;
  char swap;
  int p_size = 8/num_processes; // p_size is the number of files that each process deals with
  char filename[8];
  char* buff = calloc(input_size/8 + 1, sizeof(char));
  hex1 = calloc(digits + 1, sizeof(char));
  hex2 = calloc(digits + 1, sizeof(char));

  for (i = 0; i < digits; i++) { // init
    hex1[i] = '0';
    hex2[i] = '0';
  }
  
  for (i = pid * p_size; i < (pid + 1) * p_size; i++) {
    sprintf(filename, "input.%d", i);
    
    f = fopen(filename, "r");

    fgets(buff, input_size, f); // will read until first \n
    memcpy(hex1 + i * input_size/8, buff, strlen(buff));

    fgets(buff, input_size, f); // will read until EOF
    memcpy(hex2 + i * input_size/8, buff, strlen(buff));

    fclose(f);
  }

  for (i = 0; i < digits && i < digits - 1 - i; i++) {
    swap = hex1[digits - 1 - i];
    hex1[digits - 1 - i] = hex1[i];
    hex1[i] = swap;

    swap = hex2[digits - 1 - i];
    hex2[digits - 1 - i] = hex2[i];
    hex2[i] = swap;
  }

  hex1[digits] = '\0';
  hex2[digits] = '\0';

  free(buff);
}


void step1(int pid, int num_processes)
{
  int p_size = bits/num_processes;
  for(int i = pid * p_size; i < (pid + 1) * p_size; i++)
    {
      gi[i] = bin1[i] & bin2[i];
      pi[i] = bin1[i] | bin2[i];
    }
}


void step2(int pid, int num_processes) {
  int p_size = ngroups/num_processes;
  for(int j = pid * num_processes; j < (pid + 1) * p_size; j++)
    {
      int jstart = j*block_size;
      int* ggj_group = grab_slice(gi,jstart,block_size);
      int* gpj_group = grab_slice(pi,jstart,block_size);

      int sum = 0;
      for(int i = 0; i < block_size; i++)
	{
	  int mult = ggj_group[i]; //grabs the g_i term for the multiplication
	  for(int ii = block_size-1; ii > i; ii--)
	    {
	      mult &= gpj_group[ii]; //grabs the p_i terms and multiplies it with the previously multiplied stuff (or the g_i term if first round)
	    }
	  sum |= mult; //sum up each of these things with an or
	}
      ggj[j] = sum;

      int mult = gpj_group[0];
      for(int i = 1; i < block_size; i++)
	{
	  mult &= gpj_group[i];
	}
      gpj[j] = mult;
    }
}

void step3(int* lvlg, int* lvlp, int* lvlupg, int* lvlupp, int size, int pid, int num_processes)
{
  if (EnableBarrier == 1) MPI_Barrier(MPI_COMM_WORLD);
  int p_size = size/num_processes;
  for(int k = pid * p_size; k < (pid + 1) * p_size; k++)
    {
      int kstart = k*block_size;
      int* sgk_group = grab_slice(lvlupg,kstart,block_size);
      int* spk_group = grab_slice(lvlupp,kstart,block_size);

      int sum = 0;
      for(int i = 0; i < block_size; i++)
	{
	  int mult = sgk_group[i];
	  for(int ii = block_size-1; ii > i; ii--)
	    {
	      mult &= spk_group[ii];
	    }
	  sum |= mult;
	}
      lvlg[k] = sum;

      int mult = spk_group[0];
      for(int i = 1; i < block_size; i++)
	{
	  mult &= spk_group[i];
	}
      lvlp[k] = mult;
    }
}

void step4(int* lvlc, int* lvlg, int* lvlp, int size, int pid, int num_processes)
{
  MPI_Status status;
  if (EnableBarrier == 1) MPI_Barrier(MPI_COMM_WORLD);
  int p_size = size/num_processes;
  for (int k = pid * p_size; k < (pid + 1) * p_size; k++) {
    int scklast;
    if(k == 0) {
      scklast = 0;
    }
    else if (k % p_size != 0) {
      scklast = lvlc[k-1];
    }
    else {
      // pid 0 will never call this because they will enter the if k == 0
      MPI_Recv(&scklast, 1, MPI_INT, pid - 1, 0, MPI_COMM_WORLD, &status);
    }
    
    lvlc[k] = lvlg[k] | (lvlp[k]&scklast);
    if (k == (pid + 1) * p_size - 1 && pid != num_processes - 1) { // send to next level
      MPI_Send(&lvlc[k], 1, MPI_INT, pid + 1, 0, MPI_COMM_WORLD);
    }
  }
}

void step5(int pid, int num_processes)
{
  MPI_Status status;
  if (EnableBarrier == 1) MPI_Barrier(MPI_COMM_WORLD);
  int p_size = ngroups/num_processes;
  for (int j = pid * p_size; j < (pid + 1) * p_size; j++) {
    int gcjlast;
    if (j == 0) {
      gcjlast = 0;
    }
    else if(j%block_size == block_size-1) {
      gcjlast = sck[j/block_size];
    }
    else if (j % p_size != 0) {
      gcjlast = gcj[j-1];
    }
    else {
      // pid 0 will never call this because they will enter the if k == 0
      MPI_Recv(&gcjlast, 1, MPI_INT, pid - 1, 0, MPI_COMM_WORLD, &status);
    }
    
    gcj[j] = ggj[j] | (gpj[j]&gcjlast);
    if (j == (pid + 1) * p_size - 1 && pid != num_processes - 1) { // send to next level
      MPI_Send(&gcj[j], 1, MPI_INT, pid + 1, 0, MPI_COMM_WORLD);
    }
  }
}

void step6(int pid, int num_processes)
{
  MPI_Status status;
  if (EnableBarrier == 1) MPI_Barrier(MPI_COMM_WORLD);
  int p_size = bits/num_processes;
  for(int i = pid * p_size; i < (pid + 1) * p_size; i++) {
    int clast;
    if (i == 0) {
      clast = 0;
    }
    else if(i%block_size==block_size-1) {
      clast = gcj[i/block_size];
    }
    else if (i % p_size != 0) {
      clast = ci[i-1];
    }
    else {
      MPI_Recv(&clast, 1, MPI_INT, pid - 1, 0, MPI_COMM_WORLD, &status);
      ci[i-1] = clast; // we save this here so that we can avoid having to receive this value again in step7
    }
      
    ci[i] = gi[i] | (pi[i]&clast);
    if (i == (pid + 1) * p_size - 1 && pid != num_processes - 1) { // send to next level
      MPI_Send(&ci[i], 1, MPI_INT, pid + 1, 0, MPI_COMM_WORLD);
    } 
  }
}

void step7(int pid, int num_processes)
{
  int p_size = bits/num_processes;
  for(int i = pid * p_size; i < (pid + 1) * p_size; i++) {
    int clast;
    if(i == 0) {
      clast = 0;
    }
    else {
      clast = ci[i-1];
    }
    sumi[i] = bin1[i] ^ bin2[i] ^ clast;
  }
}

// Here, now that sum is constructed, we can ship everything to pid 0 for writing 
void combineSum(int pid, int num_processes) {
  MPI_Status status;
  if (EnableBarrier == 1) MPI_Barrier(MPI_COMM_WORLD);
  if (pid == 0) { // recieve
    for (int i = 1; i < num_processes; i++) {
      MPI_Recv(sumi + bits * i/num_processes, bits/num_processes, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
    }
  }
  else { // send
    MPI_Send(sumi + bits * pid/num_processes, bits/num_processes, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }
}

void cla(int pid, int num_processes)
{
  step1(pid, num_processes);
  step2(pid, num_processes);
  step3(sgk, spk, ggj, gpj, s1, pid, num_processes);
  step3(s2gl, s2pl, sgk, spk, s2, pid, num_processes);
  step3(s3gm, s3pm, s2gl, s2pl, s3, pid, num_processes);
  step3(s4gn, s4pn, s3gm, s3pm, s4, pid, num_processes);
  step3(s5go, s5po, s4gn, s4pn, block_size, pid, num_processes);
  step4(s5co, s5go, s5po, block_size, pid, num_processes);
  step4(s4cn, s4gn, s4pn, s4, pid, num_processes);
  step4(s3cm, s3gm, s3pm, s3, pid, num_processes);
  step4(s2cl, s2gl, s2pl, s2, pid, num_processes);
  step4(sck, sgk, spk, s1, pid, num_processes);
  step5(pid, num_processes);
  step6(pid, num_processes);
  step7(pid, num_processes);
  combineSum(pid, num_processes);
}

// testing function that just verifies my output to make sure it's correct
void verifyResult(char* hexSum) {
  FILE* f;
  int i = 0;
  char* buffer = calloc(strlen(hexSum) + 1, sizeof(char));
  f = fopen(REALOUTPUT_FILENAME, "r"); // defined in a #define at the top
  fgets(buffer, input_size, f);
  for (i = 0; i < strlen(hexSum); i++) {
    if (buffer[i] != hexSum[i]) {
      printf("%d: %c vs %c\n", i, buffer[i], hexSum[i]);
      break;
    }
  }
  fclose(f);
  free(buffer);
}

// writes output.0 thru output.7
void printResultToFiles(char *hexSum) {
  FILE *f;
  int i;
  char* buffer = calloc(strlen(hexSum)/8 + 1, sizeof(char));
  char* filename = calloc(9, sizeof(char));
  for (i = 0; i < 8; i++) {
    sprintf(filename, "output.%d", i);
    f = fopen(filename, "w");
    memcpy(buffer, hexSum + i * strlen(hexSum)/8, strlen(hexSum)/8);
    buffer[strlen(hexSum)/8] = '\0';
    fputs(buffer, f);
    fclose(f);
  }
}

int main(int argc, char *argv[]) {
  clock_t start, end;
  int num_processes, pid;
  char* hexa;
  char* hexb;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);
  MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
  
  read_input(pid, num_processes); // fills hex1, hex2 with relevant input files

  hexa = hex1; // note: does not prepend for overflow because that messes with the accuracy of the program for 8 files
  hexb = hex2; // (inputsize + 1) * 4 / 8 is not an integer!!
  // hexa = prepend_non_sig_zero(hex1);
  // hexb = prepend_non_sig_zero(hex2);
  hexa[digits] = '\0';
  hexb[digits] = '\0';
  
  bin1 = gen_formated_binary_from_hex(hexa);
  bin2 = gen_formated_binary_from_hex(hexb);

  start = clock();
  cla(pid, num_processes); // run the actual program
  end = clock();

  if (pid == 0) { // main rank does all the work at the end
    char* hexSum = revbinary_to_hex(int_to_string(sumi,bits),bits);
    reverse_string(hexSum, strlen(hexSum));
    // verifyResult(hexSum);
    printf("%s", hexSum);
    printf("\n%f s elapsed\n", (double) (end - start)/CLOCKS_PER_SEC);
    printResultToFiles(hexSum);
  }
  
  MPI_Finalize();
  return 1;
}
