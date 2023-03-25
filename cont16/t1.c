#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

volatile sig_atomic_t value = 0;
volatile sig_atomic_t finish_trigger = 0;

void SIGUSR1Handler() {
  ++value;
  printf("%d ", value);
  fflush(stdout);
}

void SIGUSR2Handler() {
  value *= -1;
  printf("%d ", value);
  fflush(stdout);
}

void SIGINTHandler() {
  finish_trigger = 1;
}

void SIGTERMHandler() {
  finish_trigger = 1;
}

int main() {
  struct sigaction USR1_handler;
  memset(&USR1_handler, 0, sizeof(USR1_handler));
  USR1_handler.sa_flags = SA_RESTART;
  USR1_handler.sa_handler = SIGUSR1Handler;
  sigaction(SIGUSR1, &USR1_handler, NULL);

  struct sigaction USR2_handler;
  memset(&USR2_handler, 0, sizeof(USR2_handler));
  USR2_handler.sa_flags = SA_RESTART;
  USR2_handler.sa_handler = SIGUSR2Handler;
  sigaction(SIGUSR2, &USR2_handler, NULL);

  struct sigaction SIGINT_handler;
  memset(&SIGINT_handler, 0, sizeof(SIGINT_handler));
  SIGINT_handler.sa_flags = SA_RESTART;
  SIGINT_handler.sa_handler = SIGINTHandler;
  sigaction(SIGINT, &SIGINT_handler, NULL);

  struct sigaction SIGTERM_handler;
  memset(&SIGTERM_handler, 0, sizeof(SIGTERM_handler));
  SIGTERM_handler.sa_flags = SA_RESTART;
  SIGTERM_handler.sa_handler = SIGTERMHandler;
  sigaction(SIGTERM, &SIGTERM_handler, NULL);

  printf("%d\n", getpid());
  fflush(stdout);
  scanf("%d", &value);

  while (finish_trigger != 1) {
    pause();
  }
  return 0;
}
