/*
  Serial CLA adder by Darwin Ding
  Compiled with gcc -Wall, no warnings
  Usage: ./a.out (requires existence of properly formatted input.0 -> input.7 in reverse, split order)

  Runtime: .081851 s on sample input
 */

#include "cla.h"
#include <time.h>

#define input_size 524288
#define block_size 8
#define verbose 0

#define digits (input_size+1)
#define bits digits*4
#define ngroups bits/block_size
#define s1 ngroups/block_size
#define s2 s1/block_size
#define s3 s2/block_size
#define s4 s3/block_size
// s5 == block_size

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

void read_input(){
  FILE* f;
  int i, swap;
  char filename[8];
  char* buff = calloc(input_size/8 + 1, sizeof(char));
  hex1 = calloc(digits, sizeof(char));
  hex2 = calloc(digits, sizeof(char));
  
  for (i = 0; i < 8; i++) {
    sprintf(filename, "input.%d", i);
    
    f = fopen(filename, "r");

    fgets(buff, input_size, f); // will read until first \n
    memcpy(hex1 + i * input_size/8, buff, strlen(buff));

    fgets(buff, input_size, f); // will read until EOF
    memcpy(hex2 + i * input_size/8, buff, strlen(buff));

    fclose(f);
  }

  for (i = 0; i < digits - 1 && i < digits - 2 - i; i++) {
    swap = hex1[digits - 2 - i];
    hex1[digits - 2 - i] = hex1[i];
    hex1[i] = swap;

    swap = hex2[digits - 2 - i];
    hex2[digits - 2 - i] = hex2[i];
    hex2[i] = swap;
  }

  hex1[digits - 1] = '\0';
  hex2[digits - 1] = '\0';
  
  free(buff);
}


void step1()
{
  for(int i = 0; i < bits; i++)
    {
      if (i % (bits/8) == 0) {
	printf("%d: %d, %d\n", i, bin1[i], bin2[i]);
      }
      gi[i] = bin1[i] & bin2[i];
      pi[i] = bin1[i] | bin2[i];
    }
}


void step2()
{
  for(int j = 0; j < ngroups; j++)
    {
      int jstart = j*block_size;
      int* ggj_group = grab_slice(gi,jstart,block_size);
      int* gpj_group = grab_slice(pi,jstart,block_size);

      int sum = 0;
      for(int i = 0; i < block_size; i++)
	{
	  int mult = ggj_group[i]; //grabs the g_i term for the multiplication
	  //            printf("g%i",i);
	  for(int ii = block_size-1; ii > i; ii--)
	    {
	      //                printf("p%i",ii);
	      mult &= gpj_group[ii]; //grabs the p_i terms and multiplies it with the previously multiplied stuff (or the g_i term if first round)
	    }
	  //            printf(" + ");
	  sum |= mult; //sum up each of these things with an or
	}
      ggj[j] = sum;

      //        printf("\n");
      int mult = gpj_group[0];
      for(int i = 1; i < block_size; i++)
	{
	  mult &= gpj_group[i];
	}
      gpj[j] = mult;
    }
}

void step3(int* lvlg, int* lvlp, int* lvlupg, int* lvlupp, int size)
{
  int sections = size;
  for(int k = 0; k < sections; k++)
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

void step4(int* lvlc, int* lvlg, int* lvlp, int size)
{
  int sections = size;
  for(int k = 0; k < sections; k++)
    {
      int scklast;
      if(k==0)
	{
	  scklast = 0;
	}
      else
	{
	  scklast = lvlc[k-1];
	}

      lvlc[k] = lvlg[k] | (lvlp[k]&scklast);
    }
}

void step5()
{
  for(int j = 0; j < ngroups; j++)
    {
      int gcjlast;
      if(j%block_size==block_size-1)
	{
	  gcjlast = sck[j/block_size];
	}
      else
	{
	  gcjlast = gcj[j-1];
	}

      gcj[j] = ggj[j] | (gpj[j]&gcjlast);
    }
}

void step6()
{
  for(int i = 0; i < bits; i++)
    {
      int clast;
      if(i%block_size==block_size-1)
	{
	  clast = gcj[i/block_size];
	}
      else
	{
	  clast = ci[i-1];
	}

      ci[i] = gi[i] | (pi[i]&clast);
      if ((i+1) % (bits/4) == 0) {
	// printf("%d, %d, %d, %d\n", gi[i], pi[i], clast, i);
      }
    }
}

void step7()
{
  for(int i = 0; i < bits; i++)
    {
      int clast;
      if(i==0)
	{
	  clast = 0;
	}
      else
	{
	  clast = ci[i-1];
	}
      sumi[i] = bin1[i] ^ bin2[i] ^ clast;
    }
}

void cla()
{
  step1();
  step2();
  step3(sgk, spk, ggj, gpj, s1);
  step3(s2gl, s2pl, sgk, spk, s2);
  step3(s3gm, s3pm, s2gl, s2pl, s3);
  step3(s4gn, s4pn, s3gm, s3pm, s4);
  step3(s5go, s5po, s4gn, s4pn, block_size);
  step4(s5co, s5go, s5po, block_size);
  step4(s4cn, s4gn, s4pn, s4);
  step4(s3cm, s3gm, s3pm, s3);
  step4(s2cl, s2gl, s2pl, s2);
  step4(sck, sgk, spk, s1);
  step5();
  step6();
  step7();
}

int main(int argc, char *argv[]) {
  clock_t start, end;
  char* hexa;
  char* hexb;

  read_input(); // fills hex1, hex2 with input.0 -> input.7
  
  hexa = prepend_non_sig_zero(hex1);
  hexb = prepend_non_sig_zero(hex2);
  hexa[digits] = '\0';
  hexb[digits] = '\0';

  bin1 = gen_formated_binary_from_hex(hexa);
  bin2 = gen_formated_binary_from_hex(hexb);

  start = clock();
  cla();
  end = clock();
  
  char* hexSum = revbinary_to_hex(int_to_string(sumi,bits),bits);
  
  reverse_string(hexSum, strlen(hexSum));
  if (hexSum[strlen(hexSum) - 1] == '0') { // if we didn't overflow, let's not append the 0 (makes it diff from example output)
    hexSum[strlen(hexSum) - 1] = '\0';
  }
  // printf("%s\n",hexSum);

  // printf("Serial CLA-adder took: %f s to run\n", (double) (end - start)/(CLOCKS_PER_SEC));
  
  return 1;
}
