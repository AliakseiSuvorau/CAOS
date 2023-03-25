#include <stdio.h>
#include <stdint-gcc.h>
#include <stdlib.h>
#include <pthread.h>

struct Data{
  int64_t begin;
  int64_t end;
  uint32_t num_of_prime;
  int64_t prime_number;
  pthread_cond_t* cond_var_inner;
  pthread_cond_t* cond_var_outer;
  pthread_mutex_t* mut_ex;
};

int8_t IsPrime(int64_t number) {
  for (int64_t i = 2; i * i <= number; ++i) {
    if (number % i == 0) {
      return 0;
    }
  }
  return 1;
}

static void* GeneratePrime(void* d) {
  struct Data* data = (struct Data*)d;
  int64_t current = data->begin;
  uint32_t count = 0;
  while (count < data->num_of_prime && current < data->end) {
    while (!IsPrime(current)) {
      ++current;
    }
    data->prime_number = current;
    ++current;
    ++count;
    pthread_cond_signal(data->cond_var_inner);
    pthread_cond_wait(data->cond_var_outer, data->mut_ex);
  }
  return NULL;
}

void FillStruct(struct Data* data,
                int64_t begin,
                int64_t end,
                uint32_t num_of_prime,
                pthread_cond_t* cond_var_inner,
                pthread_cond_t* cond_var_outer,
                pthread_mutex_t* mut_ex) {
  data->begin = begin;
  data->end = end;
  data->num_of_prime = num_of_prime;
  data->cond_var_inner = cond_var_inner;
  data->cond_var_outer = cond_var_outer;
  data->mut_ex = mut_ex;
}

int main(int argc, char** argv) {
  int64_t A = strtoll(argv[1], NULL, 10);
  int64_t B = strtoll(argv[2], NULL, 10);
  uint32_t N = strtol(argv[3], NULL, 10);

  pthread_cond_t cond_var_inner = PTHREAD_COND_INITIALIZER;
  pthread_cond_t cond_var_outer = PTHREAD_COND_INITIALIZER;
  pthread_mutex_t mut_ex = PTHREAD_MUTEX_INITIALIZER;
  size_t num_of_prime = 0;
  struct Data data;
  FillStruct(&data, A, B, N, &cond_var_inner, &cond_var_outer, &mut_ex);

  pthread_t secondary_thread;
  pthread_mutex_lock(&mut_ex);
  pthread_create(&secondary_thread, NULL, &GeneratePrime, (void*)&data);
  while (num_of_prime < N) {
    pthread_cond_wait(&cond_var_inner, &mut_ex);
    printf("%ld ", data.prime_number);
    ++num_of_prime;
    pthread_cond_signal(&cond_var_outer);
  }
  pthread_mutex_unlock(&mut_ex);
  pthread_join(secondary_thread, NULL);
  return 0;
}
