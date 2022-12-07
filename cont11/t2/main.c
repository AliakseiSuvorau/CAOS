#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
  const int kMaxLength = 4096;
  int counter = 0;
  char word[kMaxLength];
  pid_t pid;
  int flag = 1;
  while (flag) {
    pid = fork();
    if (pid == 0) {
      exit(scanf("%s", word) != EOF);
    } else {
      int status;
      waitpid(pid, &status, 0);
      int child_ret = WEXITSTATUS(status);
      if (child_ret) {
        ++counter;
      } else {
        flag = child_ret;
      }
    }
  }
  printf("%d\n", counter);
  return 0;
}
