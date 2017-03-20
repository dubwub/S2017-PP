/*
  Serial ripple carry adder by Darwin Ding
  Compiled with gcc -Wall, no warnings
  Usage: ./a.out (requires existence of properly formatted input.0 -> input.7 in reverse, split order)

  Runtime: .081851 s on sample input
 */

#include "cla.h"
#include <time.h>

#define input_size 524288
#define digits (input_size+1)
#define bits digits*4
// s5 == block_size

//Global definitions of the various arrays used in steps for easy access
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


void rippleCarryAdd()
{
  for(int i = 0; i < bits; i++) {
    int clast = 0;
    if (i > 0) {
      clast = ci[i-1];
    }
    ci[i] = (bin1[i] & bin2[i]) | (bin1[i] & clast) | (bin2[i] & clast);
    sumi[i] = bin1[i] ^ bin2[i] ^ ci[i];
  }
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
  rippleCarryAdd();
  end = clock();
  
  char* hexSum = revbinary_to_hex(int_to_string(sumi,bits),bits);
  
  reverse_string(hexSum, strlen(hexSum));
  if (hexSum[strlen(hexSum) - 1] == '0') { // if we didn't overflow, let's not append the 0 (makes it diff from example output)
    hexSum[strlen(hexSum) - 1] = '\0';
  }
  printf("%s\n",hexSum);

  printf("Ripple adder took: %f s to run\n", (double) (end - start)/(CLOCKS_PER_SEC));
  
  return 1;
}
