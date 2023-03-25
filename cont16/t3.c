#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

volatile sig_atomic_t finish_trigger = 0;

void SIGRTHandler(int sig_num, siginfo_t* info, void* context) {
  int number = info->si_int;
  if (number == 0) {
    finish_trigger = 1;
  } else {
    --number;
    union sigval ret_value;
    ret_value.sival_int = number;
    sigqueue(info->si_pid, sig_num, ret_value);
    printf("%d ", number);
    fflush(stdout);
  }
}

int main() {
  struct sigaction SIGRTMIN_handler;
  memset(&SIGRTMIN_handler, 0, sizeof(SIGRTMIN_handler));
  SIGRTMIN_handler.sa_sigaction = SIGRTHandler;
  SIGRTMIN_handler.sa_flags = SA_SIGINFO | SA_RESTART;
  sigaction(SIGRTMIN, &SIGRTMIN_handler, NULL);

  sigset_t sig_mask;
  sigfillset(&sig_mask);
  sigdelset(&sig_mask, SIGRTMIN);
  sigprocmask(SIG_SETMASK, &sig_mask, NULL);

  printf("%d\n", getpid());
  fflush(stdout);

  while (finish_trigger != 1) {
    pause();
  }
  return 0;
}
