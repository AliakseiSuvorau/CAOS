#include <sys/syscall.h>

long syscall(long number, ...);

int _start() {
  char input_symbol = 'F';
  char* input_ptr = &input_symbol;
  while(syscall(SYS_read, 0, input_ptr, 1)) {
    syscall(SYS_write, 1, input_ptr, 1);
  }
  syscall(SYS_exit, 0);
}
