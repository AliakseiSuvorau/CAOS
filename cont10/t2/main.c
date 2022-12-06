#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>

void PutInCell(int i, int j, int value, char* begin, int n, int w) {
  char* string = (char*)malloc(w * sizeof(char));
  sprintf(string, "%*d", w, value);
  int offset = (n * w + 1) * i + j * w;
  for (int k = 0; k < w; ++k) {
    begin[offset + k] = string[k];
  }
  free(string);
}

void FillRight(char* begin, int n, int w, int* i, int* j, int* cnt) {
  if (*cnt > n * n) {
    return;
  }
  while (*j < n - *i) {
    PutInCell(*i, *j, *cnt, begin, n, w);
    ++(*cnt);
    ++(*j);
  }
  --(*j);
  ++(*i);
}

void FillDown(char* begin, int n, int w, int* i, int* j, int* cnt) {
  if (*cnt > n * n) {
    return;
  }
  while (*i <= *j) {
    PutInCell(*i, *j, *cnt, begin, n, w);
    ++(*cnt);
    ++(*i);
  }
  --(*i);
  --(*j);
}

void FillLeft(char* begin, int n, int w, int* i, int* j, int* cnt) {
  if (*cnt > n * n) {
    return;
  }
  while (*j >= n - *i - 1) {
    PutInCell(*i, *j, *cnt, begin, n, w);
    ++(*cnt);
    --(*j);
  }
  ++(*j);
  --(*i);
}

void FillUp(char* begin, int n, int w, int* i, int* j, int* cnt) {
  if (*cnt > n * n) {
    return;
  }
  while (*i > *j) {
    PutInCell(*i, *j, *cnt, begin, n, w);
    ++(*cnt);
    --(*i);
  }
  ++(*i);
  ++(*j);
}

void MakeSpiral(char* begin, int n, int w) {
  int i = 0;
  int j = 0;
  int cnt = 1;
  while (cnt <= n * n) {
    FillRight(begin, n, w, &i, &j, &cnt);
    FillDown(begin, n, w, &i, &j, &cnt);
    FillLeft(begin, n, w, &i, &j, &cnt);
    FillUp(begin, n, w, &i, &j, &cnt);
  }
}

void PutEndOfLines(char* begin, int n, int w) {
  int length_of_line = n * w + 1;
  char* offset = begin + n * w;
  for (int i = 0; i < n - 1; ++i) {
    *(offset + length_of_line * i) = '\n';
  }
  *(begin + n * n * w + n - 1) = '\n';
}

int main(int argc, char** argv) {
  char* filename = argv[1];
  int output_fd = open(filename, O_CREAT | O_RDWR, 0640);
  int N = (int)strtol(argv[2], NULL, 10);  // сторона матрицы
  int W = (int)strtol(argv[3], NULL, 10);  // ширина клетки
  lseek(output_fd, N * N * W + N - 1, SEEK_SET);
  write(output_fd, "", 1);
  if (output_fd == -1) {
    close(output_fd);
    return 1;
  }
  char* begin = mmap(NULL, N * N * W + N, PROT_READ | PROT_WRITE, MAP_SHARED, output_fd, 0);
  MakeSpiral(begin, N, W);
  PutEndOfLines(begin, N, W);
  munmap(begin, N * N * W + N);
  close(output_fd);
  return 0;
}
