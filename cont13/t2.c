#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int number;
int stop = 0;
int start = 1;
int sv[2];

static void* Decrease(void* arg) {
  int cur = 0;
  while(1) {
    if (stop == 1) {
      return NULL;
    }
    // считываем первое или последующее значение изменяемого числа
    if (start == 1) {
      start = 0;
      cur = number;
    } else {
      read(sv[1], &cur, sizeof(int));
    }
    cur -= 3;
    if (cur == 0 || cur > 100) {
      printf("%d\n", cur);
      stop = 1;
      write(sv[1], &cur, sizeof(int));
      return NULL;
    } else {
      if(stop == 1) {
        return NULL;
      }
      printf("%d\n", cur);
      write(sv[1], &cur, sizeof(int));
    }
  }
}

static void* Increase(void* arg) {
  int cur = 0;
  while(1) {
    if (stop == 1) {
      return NULL;
    }
    read(sv[0], &cur, sizeof(int));
    cur += 5;
    if (cur == 0 || cur > 100) {
      printf("%d\n", cur);
      stop = 1;
      write(sv[0], &cur, sizeof(int));
      return NULL;
    } else {
      if(stop == 1) {
        return NULL;
      }
      printf("%d\n", cur);
      write(sv[0], &cur, sizeof(int));
    }
  }
}

int main(int argc, char** argv) {
  number = (int)strtol(argv[1], NULL, 10);
  socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
  pthread_t inc_thread, dec_thread;
  pthread_create(&dec_thread, NULL, Decrease, 0);
  pthread_create(&inc_thread, NULL, Increase, 0);
  pthread_join(dec_thread, NULL);
  pthread_join(inc_thread, NULL);
  return 0;
}
