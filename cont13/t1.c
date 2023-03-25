#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>

static void* part_sum(void* arg) {
  int64_t number = 0;
  int64_t part_sum = 0;
  while (scanf("%ld", &number) != -1) {
    part_sum += number;
  }
  return (void*)part_sum;
}

int main(int argc, char** argv) {
  long num_of_threads = strtol(argv[1], NULL, 10);
  pthread_t* array_of_threads = (pthread_t*)calloc(num_of_threads, sizeof(pthread_t));
  pthread_attr_t attributes;
  pthread_attr_init(&attributes);
  pthread_attr_setguardsize(&attributes, 0);
  pthread_attr_setstacksize(&attributes, PTHREAD_STACK_MIN);
  for (long i = 0; i < num_of_threads; ++i) {
    pthread_create(array_of_threads + i, &attributes, part_sum, 0);
  }
  int64_t sum = 0;
  for (long i = 0; i < num_of_threads; ++i) {
    void* part_of_sum = NULL;
    pthread_join(array_of_threads[i], &part_of_sum);
    sum += (int64_t)part_of_sum;
  }
  printf("%ld", sum);
  pthread_attr_destroy(&attributes);
  free(array_of_threads);
  return 0;
}
