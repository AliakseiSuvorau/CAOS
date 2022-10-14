#include <stdio.h>
#include <wchar.h>
#include <errno.h>
#include <locale.h>

int main() {
  setlocale(LC_ALL, "");
  wchar_t c;
  int utf8 = 0;
  int not_utf8 = 0;
  while (scanf("%C", &c) != -1 && errno != EILSEQ) {
    if (c >> 8 == 0) {
      utf8++;
    } else {
      not_utf8++;
    }
  }
  printf("%d %d ", utf8, not_utf8);
  if (errno == EILSEQ) {
    return 1;
  }
  return 0;
}
