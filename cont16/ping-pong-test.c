#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

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

  pid_t sig_pid = /*pid_of_main*/;
  int number = 10;
  union sigval ret_value;
  ret_value.sival_int = number;
  sigqueue(sig_pid, SIGRTMIN, ret_value);

  while (finish_trigger != 1) {
    pause();
  }
  return 0;
}
