#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

double eps = 0.00000000000000000001;

void change_places(double** arr, int str_num, int n) {
  double tmp = 0.;
  for (int i = 0; i < n + 1; ++i) {
    tmp = arr[str_num][i];
    arr[str_num][i] = arr[str_num + 1][i];
    arr[str_num = 1][i] = tmp;
  }
}

double* solve(double** arr, int n) {
  double* sol = (double*)malloc(sizeof(double) * (n));
  for (int i = 0; i < n; ++i) {
    for (int j = i + 1; j < n; ++j) {
      if (arr[i][i] <= eps && arr[i][i] >= -eps) {
        change_places(arr, i, n);
      }
      double ratio = arr[j][i] / arr[i][i];
      int k = i;
      for (; k < (n + 1) - 4; k += 4) {
        double ratio_arr[4] = {ratio, ratio, ratio, ratio};
        __m256d ymm_array = _mm256_loadu_pd(arr[i] + k);
        __m256d ymm_ratio = _mm256_loadu_pd(ratio_arr);
        __m256d ymm_tmp = _mm256_mul_pd(ymm_array, ymm_ratio);
        _mm256_storeu_pd(ratio_arr, ymm_tmp);
      }
      while (k < n + 1) {
        double tmp = ratio * arr[i][k];
        arr[j][k] -= tmp;
        ++k;
      }
    }
  }
  for (int i = n - 1; i >= 0; --i) {
    double sum = 0.;
    int j = i + 1;
    for (; j < n - 4; j += 4) {
      __m256d ymm_array = _mm256_loadu_pd(arr[i] + j);
      __m256d ymm_sol = _mm256_loadu_pd(sol + j);
      __m256d ymm_tmp = _mm256_mul_pd(ymm_array, ymm_sol);
      double sum_tmp[4];
      _mm256_storeu_pd(sum_tmp, ymm_tmp);
      sum += (sum_tmp[0] + sum_tmp[1] + sum_tmp[2] + sum_tmp[3]);
    }
    while (j < n) {
      sum += (arr[i][j] * sol[j]);
      ++j;
    }
    double numerator = arr[i][n] - sum;
    sol[i] = numerator / arr[i][i];
  }
  return sol;
}

int main() {
  int n = 0;
  scanf("%d", &n);
  double** arr = (double**)malloc(sizeof(double*) * n);
  for (int i = 0; i < n; ++i) {
    arr[i] = (double*)malloc(sizeof(double) * (n + 1));
  }
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n + 1; ++j) {
      double elem = 0.;
      scanf("%lf", &elem);
      arr[i][j] = elem;
    }
  }
  double* sol = solve(arr, n);
  for (int i = 0; i < n; ++i) {
    printf("%lf ", sol[i]);
  }
  for (int i = 0; i < n; ++i) {
    free(arr[i]);
  }
  free(arr);
  free(sol);
  return 0;
}
