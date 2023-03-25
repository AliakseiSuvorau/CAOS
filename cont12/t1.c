#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main() {
  char string[4096];
  char program[4096];
  fgets(string, sizeof(string), stdin);
  char* last_sym = strchr(string, '\n');
  if (last_sym != NULL) {
    *last_sym = '\0';
  }
  if (strlen(string) == 0) {
    return 0;
  }
  char text_of_program[4096] = "#include <stdio.h>\nint main() {\nint ans = (%s);\nprintf(\"%%d\", ans);\nreturn 0;\n}\0";
  snprintf(program, sizeof(program), text_of_program, string);
  int fd = open("solution.c", O_RDWR | O_CREAT | O_TRUNC, 0770);
  write(fd, program, strlen(program));
  close(fd);
  pid_t pid = fork();
  if (pid == 0) {
    execlp("gcc", "gcc", "-o", "sol", "solution.c", NULL);
    return 0;
  }
  int status;
  waitpid(pid, &status, 0);
  execlp("./sol", "./sol", NULL);
  return 0;
}
