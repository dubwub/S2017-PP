#ifndef CLA_512_MAIN_C_H
#define CLA_512_MAIN_C_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


int rand_in_range(int min, int max);
char hexLookup(int in);
char* generate_random_hex(size_t len);
char* hex_to_binary_single(char theDigit);
char* hex_to_binary(char* hexString);
void reverse_string(char* str, size_t len);
int* grab_slice(int* input, int starti, int length);
char* grab_slice_char(char* input, int starti, int length);
int* string_to_int(char* str);
char* int_to_string(int* in, size_t len);
char binary_quad_to_hex_single(char *binary);
char* revbinary_to_hex(char* binaryLine,int len);
int* gen_formated_binary_from_hex(char *hex);
void print_chararrayln(char *in);

int rand_in_range(int min, int max)
{
  return (rand() % (max - min) + min);
}

char hexLookup(int in)
{

  if(in < 10)
    {
      char buf[1];
      sprintf(buf,"%i",in);
      return buf[0];
    }
  else
    {
      switch(in)
	{
	case 10: return 'A';
	case 11: return 'B';
	case 12: return 'C';
	case 13: return 'D';
	case 14: return 'E';
	case 15: return 'F';
	default: return '0';
	}
    }
}

char* generate_random_hex(size_t len)
{
  char* output = calloc(len+1,sizeof(char));
  output[len] = '\0';
  for(int i = 0; i < len; i++)
    {
      output[i] = hexLookup(rand_in_range(0,16));
    }
  return output;
}

char* prepend_non_sig_zero(char* str)
{
  size_t len = strlen(str);
  char* output = calloc(len+1,sizeof(char));
  output[0] = '0';
  output[len+1] = '\0';
  for(int i = 1; i < len+1; i++)
    {
      output[i] = str[i-1];
    }
  return output;
}

int* string_to_int(char* str)
{
  size_t len = strlen(str);

  int* output = calloc(len,sizeof(int));

  for(int i = 0; i < len; i++)
    {
      output[i] = str[i] - '0';
    }

  return output;
}

char* int_to_string(int* in, size_t len)
{
  char* output = calloc(len,sizeof(char));

  int buflen = 0;
  for(int i = 0; i < len; i++)
    {
      buflen += sprintf(output+buflen, "%i",in[i]);
    }

  //
  //    for (int i = 0 ; i < len ; ++i)
  //    {
  //        output[i] = in[i] + '0';
  //    }

  return output;
}

char* hex_to_binary_single(char theDigit)
{
  char* binaryVersion;
  switch(theDigit)
    {
    case '0': binaryVersion = "0000"; break;
    case '1': binaryVersion = "0001"; break;
    case '2': binaryVersion = "0010"; break;
    case '3': binaryVersion = "0011"; break;
    case '4': binaryVersion = "0100"; break;
    case '5': binaryVersion = "0101"; break;
    case '6': binaryVersion = "0110"; break;
    case '7': binaryVersion = "0111"; break;
    case '8': binaryVersion = "1000"; break;
    case '9': binaryVersion = "1001"; break;
    case 'A': binaryVersion = "1010"; break;
    case 'B': binaryVersion = "1011"; break;
    case 'C': binaryVersion = "1100"; break;
    case 'D': binaryVersion = "1101"; break;
    case 'E': binaryVersion = "1110"; break;
    case 'F': binaryVersion = "1111"; break;
    default: binaryVersion = "0000"; break;
    }
  return binaryVersion;
}

char binary_quad_to_hex_single(char *binary)
{
  char hexVersion;
  char* buffer = calloc(4, sizeof(char));
  buffer[4] = '\0';

  long int hexInt = strtol(binary, NULL,2);

  sprintf(buffer,"%lX",hexInt);

  hexVersion = buffer[0];

  return hexVersion;

}

char* revbinary_to_hex(char* binaryLine,int len)
{
  reverse_string(binaryLine, strlen(binaryLine));

  size_t blen = (size_t)len/4 + (len%4);
  char* buffer = calloc(blen,sizeof(char));
  buffer[blen] = '\0';
  int bufferLength = 0;
  for(int j = 0; j < blen; j++)
    {
      int jstart = j*4;
      char* binquad = grab_slice_char(binaryLine, jstart,4);

      char hexDigit = binary_quad_to_hex_single(binquad);

      bufferLength += sprintf(buffer+bufferLength, "%c",hexDigit);
    }

  return buffer;
}

char* hex_to_binary(char* hexString)
{
  size_t num = strlen(hexString);
  char* buffer = calloc(num*4+1, sizeof(char));
  buffer[num*4] = '\0';
  int len = 0;

  for(int i = 0; i < num; i++)
    {
      len += sprintf(buffer+len, "%s",hex_to_binary_single(hexString[i]));
    }
  return buffer;
}

void reverse_string(char* str, size_t len)
{
  size_t i;
  char tmp;
  size_t j;

  for (i = 0, j = len - 1; i < (len / 2); i++, j--) {
    tmp = str[j];
    str[j] = str[i];
    str[i] = tmp;
  }
}

int* gen_formated_binary_from_hex(char *hex)
{
  char* binChar = hex_to_binary(hex);
  size_t len = strlen(binChar);
  reverse_string(binChar,len);
  int* bin = string_to_int(binChar);

  return bin;
}

void print_chararrayln(char *in)
{
  size_t len = strlen(in);
  for(int i = 0; i<len; i++)
    {
      printf("%c",in[i]);
    }
  printf("\n");
}

int* grab_slice(int* input, int starti, int length)
{
  int* output = calloc((size_t)length,sizeof(int));

  int i,j;
  for(i = 0, j = starti; i<length; i++,j++)
    {
      output[i] = input[j];
    }
  return output;
}

char* grab_slice_char(char* input, int starti, int length)
{
  char* output = calloc((size_t)length,sizeof(char));

  int i,j;
  for(i = 0, j = starti; i<length; i++,j++)
    {
      output[i] = input[j];
    }
  return output;
}


#endif //CLA_512_MAIN_C_H
