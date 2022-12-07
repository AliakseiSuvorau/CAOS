#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  int counter = 1;
  const int kMaxCounter = (int)strtol(argv[1], NULL, 10);
  pid_t pid;
  while (counter < kMaxCounter) {
    pid = fork();
    if (pid == 0) {
      printf("%d ", counter);
      exit(0);
    }
    ++counter;
    int status;
    waitpid(pid, &status, 0);
  }
  printf("%d\n", counter);
  return 0;
}
