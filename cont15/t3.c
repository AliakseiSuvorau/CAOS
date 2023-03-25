#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char** argv) {
  if (argc == 2) {
    execlp(argv[1], argv[1], NULL);
    perror("exec error");
    exit(1);
  }
  int fd_pair[2];
  char* cmd = argv[1];
  if (pipe(fd_pair) == -1) {
    perror("pipe error");
    exit(1);
  }
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork error");
    exit(1);
  }
  if (pid == 0) {
    dup2(fd_pair[1], 1);
    close(fd_pair[1]);
    execlp(cmd, cmd, NULL);
    perror("exec error");
    exit(1);
  }
  close(fd_pair[1]);
  pid_t pid2 = fork();
  if (pid2 == -1) {
    perror("fork error");
    exit(1);
  }
  if (pid2 == 0) {
    dup2(fd_pair[0], 0);
    close(fd_pair[0]);
    argv[1] = argv[0];
    execvp(argv[1], argv + 1);
    perror("exec error");
    exit(1);
  }
  close(fd_pair[0]);
  waitpid(pid, NULL, 0);
  waitpid(pid2, NULL, 0);
  return 0;
}
