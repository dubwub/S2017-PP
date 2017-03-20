#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define fileSize 65536

int main() {
  char testFile[fileSize+1];
  char* filename = calloc(12, sizeof(char));
  int i;
  FILE* f;
  testFile[fileSize] = '\0';

  for (i = 0; i < fileSize; i++) {
    testFile[i] = 'F';
  }
  
  for (i = 0; i < 8; i++) {
    sprintf(filename, "testinput.%d", i);
    f = fopen(filename, "w");
    fputs(testFile, f);
    putc('\n', f);
    fputs(testFile, f);
    putc('\n', f);
    fclose(f);
  }
}
