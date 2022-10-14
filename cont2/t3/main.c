#include <stdio.h>
#include <stdint.h>

int main() {
  char c;
  uint64_t set_1 = 0;
  uint64_t set_2 = 0;
  const char A_ascii = 'A' - 10;
  const char a_ascii = 'a' - 36;
  while (scanf("%c", &c) != -1) {
    switch (c)
    {
      case '&':
        set_1 &= set_2;
        set_2 = 0;
        break;
      case '|':
        set_1 |= set_2;
        set_2 = 0;
        break;
      case '^':
        set_1 ^= set_2;
        set_2 = 0;
        break;
      case '~':
        set_1 = ~set_1;
        break;
      default:
        if ('0' <= c && c <= '9') {
          set_2 |= (1ull << (c - '0'));
        } else {
          if ('A' <= c && c <= 'Z') {
            set_2 |= (1ull << (c - A_ascii));
          } else {
            set_2 |= (1ull << (c - a_ascii));
          }
        }
        break;
    }
  }
  for (int i = 0; i < 62; ++i) {
    if (i < 10 && (set_1 >> i) & 1) {
      printf("%c", '0' + i);
    } else {
      if (i > 9 && i < 36 && (set_1 >> i) & 1) {
        printf("%c", A_ascii + i);
      } else {
        if ((set_1 >> i) & 1) {
          printf("%c", a_ascii + i);
        }
      }
    }
  }
  return 0;
}
