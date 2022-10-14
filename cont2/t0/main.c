#include <stdint.h>
#include <stdio.h>

typedef uint8_t ITYPE;

extern void sum(ITYPE first, ITYPE second, ITYPE* res, int* CF) {
  (*CF) = 0;
  (*res) = 0;
  ITYPE dec = 0;
  int size = 8 * sizeof(ITYPE);
  for (int i = 0; i < size; ++i) {
    ITYPE dig_first = (first >> i) & 1;
    ITYPE dig_second = (second >> i) & 1;
    (*res) ^= ((dig_first ^ dig_second ^ dec) << i);
    dec = (dig_first & dig_second) | (dec & dig_second) | (dec & dig_first);
  }
  (*CF) = dec;
}

int main () {
  ITYPE a = 255;
  ITYPE b = 255;
  int cf = 0;
  ITYPE c = 0;
  sum(a, b, &c, &cf);
  printf("%lu %d", c, cf);
}

