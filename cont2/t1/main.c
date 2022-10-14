#include <stdint.h>

extern void mul(ITYPE first, ITYPE second, ITYPE* res, int* CF) {
  int size = 8 * sizeof(ITYPE);
  ITYPE dec = 0;
  ITYPE overflow = 0;
  *res = 0;
  *CF = 0;
  for (int i = 0; i < size; ++i) {
    ITYPE digit_second = (second >> i) & 1;
    for (int j = 0; j < size; ++j) {
      ITYPE digit_first = (first >> j) & 1;
      ITYPE res_digit = 0;
      ITYPE new_digit = 0;
      new_digit = digit_second & digit_first;
      if (i + j < size) {
        res_digit = (*res >> (i + j)) & 1;
        *res ^= ((new_digit ^ dec) << (i + j));
      }
      if (i + j >= size && (new_digit ^ dec) != 0) {
        overflow = 1;
      }
      dec = (res_digit & new_digit) | (res_digit & dec) | (new_digit & dec);
    }
  }
  (*CF) = overflow;
}

