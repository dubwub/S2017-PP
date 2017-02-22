// 512 bit carry adder
// Darwin Ding, Parallel Prog HW1
// usage: ./main num1 num2
// prints out: num1+num2

// accepts up to 128 digits for either num1, num2
// accepted digits: 0-9, a-f

#include <stdlib.h> // calloc, free
#include <stdio.h> // printf
#include <string.h> // strlen

// hex will be outputted into bin, where bin[511] is the MSB. assumes hex[0] is MSB
void getBinaryFromHex(char* hex, char* bin) {
  int iterator = 0;
  int hexValue = 0;
  int binIt = 4;
  size_t size = strlen(hex);
  if (size >= 128) {
    printf("hex value too long! memory leaks inbound, sorry! (capped to 128 digit hex value)\n");
    free(bin); // can't reach other number, gg
    exit(1);
  }
  for (iterator = 0; hex[iterator] != '\0'; iterator++) {
    if (hex[iterator] >= '0' && hex[iterator] <= '9') {
      hexValue = hex[iterator] - '0';
    }
    else if (hex[iterator] >= 'a' && hex[iterator] <= 'f') {
      hexValue = 10 + hex[iterator] - 'a';
    }
    else {
      printf("found: %c, crashing now -- sorry! memory leaks inbound (0-9, a-f are accepted)\n", hex[iterator]);
      free(bin); // can't reach other number, gg
      exit(1);
    }
    for (binIt = 4; binIt >= 1; binIt--) {
      bin[size*4 - 4*iterator - binIt] = hexValue % 2;
      hexValue >>= 1;
    }
  }
}

// simply prints a 512 byte block (just used for testing)
void printBinaryBlock(char* head) {
  int a;
  for (a = 0; a < 512; a++) {
    printf("%d", head[a]);
  }
  printf("\n");
}

// complicatedly prints a 512 byte block as a 128 digit hex number backwards
// because it's going to be passed arrays where MSB is at index 511...
void printHexBlock(char* head) {
  char startPrint = 0; // ghetto boolean
  char hex[128];
  int i;
  for (i = 0; i < 128; i++) {
    hex[i] = head[i*4] + head[i*4 + 1] * 2 + head[i*4 + 2] * 4 + head[i*4 + 3] * 8;
  }
  for(i = 127; i >= 0; i--) {
    if (hex[i] != 0) {
      startPrint = 1;
    }
    if (startPrint == 0) continue;
    if (hex[i] < 10) {
      printf("%d", hex[i]);
    }
    else {
      printf("%c", hex[i] - 10 + 'a');
    }
  }
  printf("\n");
}

// sets sum = num1 + num2
// will overwrite sum. num1, num2 will be unchanged
// reminder that for all 3 char* arrays, array[511] is MSB!!
void CLAadd(char* num1, char* num2, char* sum) {
  int i, j; // iterator
  char g[512];
  char p[512];
  char c[512];
  char gg[64];
  char gp[64];
  char gc[64];
  char sg[8];
  char sp[8];
  char sc[8];
  
  // fill g, p
  for (i = 0; i < 512; i++) {
    g[i] = num1[i] && num2[i];
    p[i] = num1[i] || num2[i];
  }

  // fill gg, gp
  // SORRY FOR THE MESS
  for (i = 0; i < 64; i++) {
    gg[i] = g[i*8+7] ||
      (g[i*8+6] && p[i*8+7]) ||
      (g[i*8+5] && p[i*8+7] && p[i*8+6]) ||
      (g[i*8+4] && p[i*8+7] && p[i*8+6] && p[i*8+5]) ||
      (g[i*8+3] && p[i*8+7] && p[i*8+6] && p[i*8+5] && p[i*8+4]) ||
      (g[i*8+2] && p[i*8+7] && p[i*8+6] && p[i*8+5] && p[i*8+4] && p[i*8+3]) ||
      (g[i*8+1] && p[i*8+7] && p[i*8+6] && p[i*8+5] && p[i*8+4] && p[i*8+3] && p[i*8+2]) ||
      (g[i*8] && p[i*8+7] && p[i*8+6] && p[i*8+5] && p[i*8+4] && p[i*8+3] && p[i*8+2] && p[i*8+1]);
    gp[i] = p[i*8] && p[i*8+1] && p[i*8+2] && p[i*8+3] && p[i*4] && p[i*5] && p[i*8+6] && p[i*8+7];
  }

  // fill sg, sp
  // SORRY FOR THE MESS
  for (i = 0; i < 8; i++) {
    sg[i] = gg[i*8+7] ||
      (gg[i*8+6] && gp[i*8+7]) ||
      (gg[i*8+5] && gp[i*8+7] && gp[i*8+6]) ||
      (gg[i*8+4] && gp[i*8+7] && gp[i*8+6] && gp[i*8+5]) ||
      (gg[i*8+3] && gp[i*8+7] && gp[i*8+6] && gp[i*8+5] && gp[i*8+4]) ||
      (gg[i*8+2] && gp[i*8+7] && gp[i*8+6] && gp[i*8+5] && gp[i*8+4] && gp[i*8+3]) ||
      (gg[i*8+1] && gp[i*8+7] && gp[i*8+6] && gp[i*8+5] && gp[i*8+4] && gp[i*8+3] && gp[i*8+2]) ||
      (gg[i*8] && gp[i*8+7] && gp[i*8+6] && gp[i*8+5] && gp[i*8+4] && gp[i*8+3] && gp[i*8+2] && gp[i*8+1]);
    sp[i] = gp[i*8] && gp[i*8+1] && gp[i*8+2] && gp[i*8+3] && gp[i*4] && gp[i*5] && gp[i*8+6] && gp[i*8+7];
  }

  // fill sc
  for (i = 0; i < 8; i++) {
    sc[i] = sg[i] || (i == 0 ? 0 : sp[i]*sg[i-1]);
  }

  // fill gc
  for (i = 0; i < 64; i++) {
    gc[i] = gg[i] || (gp[i] && ((i%8 == 0) ? sc[i/8] : gc[i-1]));
  }

  // fill c
  for (i = 0; i < 512; i++) {
    c[i] = g[i] || (p[i] && ((i%64 == 0) ? gc[i/64] : c[i-1]));
  }

  // fill sum
  for (i = 0; i < 512; i++) {
    sum[i] = num1[i] ^ num2[i] ^ (i == 0 ? 0 : c[i-1]);
  }
}

int main(int argc, char *argv[]) {
  unsigned char* num1;
  unsigned char* num2;
  unsigned char* sum;
  if (argc != 3) {
    printf("usage: %s num1 num2, prints sum\n", argv[0]);
  }
  else {
    num1 = calloc(512, sizeof(char));
    num2 = calloc(512, sizeof(char));
    sum = calloc(512, sizeof(char));
    getBinaryFromHex(argv[1], num1);
    getBinaryFromHex(argv[2], num2);
    /* printBinaryBlock(num1);
     printBinaryBlock(num2); */
    CLAadd(num1, num2, sum);
    printHexBlock(sum);
    free(num1);
    free(num2);
    free(sum);
  }
}
