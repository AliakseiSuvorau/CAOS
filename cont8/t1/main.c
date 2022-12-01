#include <stdio.h>
#include <stdint-gcc.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int in = open(argv[1], O_RDONLY);
  if (in == -1) {
    close(in);
    return 1;
  }
  uint32_t next_pointer = 1;
  int value = 0;
  while (next_pointer != 0) {
    int value_read = read(in, &value, sizeof(value));
    if (value_read <= 0) {
      close(in);
      return 0;
    }
    read(in, &next_pointer, sizeof(next_pointer));
    printf("%d ", value);
    lseek(in, next_pointer, SEEK_SET);
  }
  close(in);
  return 0;
}
