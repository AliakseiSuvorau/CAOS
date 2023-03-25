#include <signal.h>
#include <unistd.h>
#include <stdio.h>

char** argv_global;
volatile sig_atomic_t finish_trigger = 0;

void SIGRTHandler(int sig_num) {
  if (sig_num - SIGRTMIN != 0) {
    char buffer[4096];
    FILE *file = fopen(argv_global[sig_num - SIGRTMIN], "r");
    fgets(buffer, sizeof(buffer), file);
    printf("%s", buffer);
    fflush(stdout);
    fclose(file);
  } else {
    finish_trigger = 1;
  }
}

int main(int argc, char** argv) {
  argv_global = argv;
  sigset_t sig_mask;
  sigfillset(&sig_mask);
  for (int i = 0; i < argc; ++i) {
    signal(SIGRTMIN + i, SIGRTHandler);
    sigdelset(&sig_mask, SIGRTMIN + i);
  }
  sigprocmask(SIG_SETMASK, &sig_mask, NULL);

  while (finish_trigger != 1) {
    pause();
  }
  return 0;
}
