#include <sys/syscall.h>

long syscall(long number, ...);

int _start() {
  char hello_str[13] = "Hello, World!";
  syscall(SYS_write, 1, hello_str, 13);
  syscall(SYS_exit, 0);
}
