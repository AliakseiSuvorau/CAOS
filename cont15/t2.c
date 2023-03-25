#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint-gcc.h>

int main(int argc, char** argv) {
  int compile_find_pair[2];
  int find_count_pair[2];
  if (pipe(compile_find_pair) == -1) {
    perror("pipe error");
    exit(1);
  }
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork error");
    exit(1);
  }
  if (pid == 0) {
    dup2(compile_find_pair[1], 2);
    close(compile_find_pair[1]);
    execlp("gcc", "gcc", argv[1], NULL);
    perror("exec error");
    exit(1);
  }
  close(compile_find_pair[1]);
  if (pipe(find_count_pair) == -1) {
    perror("pipe error");
    exit(1);
  }
  pid_t pid2 = fork();
  if (pid2 == -1) {
    perror("fork error");
    exit(1);
  }
  if (pid2 == 0) {
    dup2(find_count_pair[1], 1);
    close(find_count_pair[1]);
    dup2(compile_find_pair[0], 0);
    close(compile_find_pair[0]);
    execlp("grep", "grep", "-E", "error|warning", NULL);
    perror("exec error");
    exit(1);
  }
  close(find_count_pair[1]);
  close(compile_find_pair[0]);
  uint64_t error_lines = 0;
  uint64_t warning_lines = 0;
  uint64_t line;
  uint64_t character;
  char type_of_line;
  char prev_type = 'q';
  uint64_t prev_line = 0;
  dup2(find_count_pair[0], 0);
  close(find_count_pair[0]);
  char format[4096] = "\0";
  strcat(format, argv[1]);
  strcat(format, ":%ld:%ld: %c%*[^\n]%*c");
  while(scanf(format, &line, &character, &type_of_line) > 0) {
    if (type_of_line == 'e') {
      if (prev_type == type_of_line && line == prev_line) {
        continue;
      }
      prev_type = type_of_line;
      prev_line = line;
      ++error_lines;
    }
    if (type_of_line == 'w') {
      if (prev_type == type_of_line && line == prev_line) {
        continue;
      }
      prev_type = type_of_line;
      prev_line = line;
      ++warning_lines;
    }
  }
  waitpid(pid, NULL, 0);
  waitpid(pid2, NULL, 0);
  printf("%ld %ld\n", error_lines, warning_lines);
  return 0;
}
