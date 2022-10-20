#include <stdio.h>
#include <stdlib.h>

void sum(size_t N, const int *A, const int *B, int *R);

int main() {
	int n = 4;
	int a[4] = {3, 2, -2, 4};
	int b[4] = {5, 0, 3, 2};
	int r[4];
	sum(n, a, b, r);
	for (int i = 0; i < n; i++) {
		printf("%d ", r[i]);
	}
	return 0;
}
