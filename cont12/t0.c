#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
  char string[4096];
  char program[4096];
  fgets(string, sizeof(string), stdin);
  char* last_sym = strchr(string, '\n');
  if (strlen(string) == 0) {
    return 0;
  }
  if (last_sym != NULL) {
    *last_sym = '\0';
  }
  sprintf(program, "ans = %s; print(ans)", string);
  execlp("python3", "python3", "-c", program, NULL);
  return 1;
}

