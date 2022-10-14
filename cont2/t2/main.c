#include <stdio.h>
#include <stdint.h>

typedef enum {
  PlusZero      = 0x00,
  MinusZero     = 0x01,
  PlusInf       = 0xF0,
  MinusInf      = 0xF1,
  PlusRegular   = 0x10,
  MinusRegular  = 0x11,
  PlusDenormal  = 0x20,
  MinusDenormal = 0x21,
  SignalingNaN  = 0x30,
  QuietNaN      = 0x31
} float_class_t;

union bit_mask {
  double number;
  uint64_t number_int;
};

extern float_class_t classify(double *value_ptr) {
  uint64_t mantissa;
  uint64_t exp;
  uint64_t sign;
  uint64_t nan;
  union bit_mask translate;
  uint64_t twelve_ones = 4095;
  uint64_t eleven_ones = 2047;
  translate.number = (*value_ptr);
  sign = ((translate.number_int) >> 63) & 1lu;
  exp = ((translate.number_int) >> 52) & eleven_ones;
  mantissa = (translate.number_int) & (~(twelve_ones << 52));
  nan = ((mantissa >> 51) & 1lu);
  if (sign == 0 && exp == 0 && mantissa == 0) {
    return PlusZero;
  }
  if (sign == 1lu && exp == 0 && mantissa == 0) {
    return MinusZero;
  }
  if (sign == 0 && exp == eleven_ones && mantissa == 0) {
    return PlusInf;
  }
  if (sign == 1lu && exp == eleven_ones && mantissa == 0) {
    return MinusInf;
  }
  if (sign == 0 && exp == 0 && mantissa != 0) {
    return PlusDenormal;
  }
  if (sign == 1lu && exp == 0 && mantissa != 0) {
    return MinusDenormal;
  }
  if (exp == eleven_ones && mantissa != 0 && nan == 1lu) {
    return QuietNaN;
  }
  if (exp == eleven_ones && mantissa != 0 && nan == 0) {
    return SignalingNaN;
  }
  if (sign == 0) {
    return PlusRegular;
  }
  return MinusRegular;
}

