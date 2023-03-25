#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Item {
  struct Item *next;
  int64_t value;
} item_t;

item_t null = {NULL, -1};
item_t* list_tail = &null;

typedef struct Data {
  long k;
  long index;
} Data;

void Insert(item_t* tail, int64_t value) {
  item_t* new_tail = (item_t*)malloc(sizeof(item_t));
  new_tail->value = value;
  while (1) {
    item_t* old_tail_next = new_tail->next = tail->next;
    if (atomic_compare_exchange_strong(&tail->next, &old_tail_next, new_tail)) {
      return;
    }
    sched_yield();
  }
}

void* ThreadFunc(void* d) {
  struct Data* data = (struct Data*)d;
  for (long k = 0; k < data->k; ++k) {
    Insert(list_tail, data->index * data->k + k);
  }
  return NULL;
}

void PrintList() {
  item_t* cur = list_tail;
  while (cur->next != NULL) {
    if (cur->value != -1) {
      printf("%ld ", cur->value);
    }
    cur = cur->next;
  }
  if (cur->value != -1) {
    printf("%ld\n", cur->value);
  }
}

void DeleteList(item_t* cur) {
  if (cur->next != NULL) {
    DeleteList(cur->next);
  }
  if (cur->value != -1) {
    free(cur);
  }
}

int main(int argc, char** argv) {
  long N = strtol(argv[1], NULL, 10);
  long k = strtol(argv[2], NULL, 10);

  pthread_t* thread_arr = (pthread_t*)calloc(N, sizeof(pthread_t));
  struct Data* data_arr = (struct Data*)calloc(N, sizeof(struct Data));
  for (long i = 0; i < N; ++i) {
    data_arr[i].k = k;
    data_arr[i].index = i;
    pthread_create(&thread_arr[i], NULL, (void* (*)(void*))ThreadFunc, &data_arr[i]);
  }
  for (long i = 0; i < N; ++i) {
    pthread_join(thread_arr[i], NULL);
  }
  PrintList();
  free(thread_arr);
  free(data_arr);
  DeleteList(list_tail);
  return 0;
}
