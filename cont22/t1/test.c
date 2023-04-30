#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char** argv) {
  int fd = open(argv[1], O_RDONLY);
  char buff[4096] = {};
  int ptr = 0;
  while(read(fd, buff + ptr, 1) > 0) {
    ++ptr;
  }
  buff[ptr] = '\0';
  write(1, buff, strlen(buff));
  return 0;
}
