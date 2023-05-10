#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef double (*function_t)(double);

int main(int argc, char *argv[]) {
  char* lib_name = argv[1];
  char* func_name = argv[2];

  void* library = dlopen(lib_name, RTLD_NOW);
  if (!library) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    exit(1);
  }

  void* func_ptr = dlsym(library, func_name);
  if (!func_ptr) {
    fprintf(stderr, "dlsym: %s\n", dlerror());
    exit(2);
  }

  function_t func = func_ptr;

  double arg;
  double res;

  while (scanf("%lf", &arg) != EOF) {
    res = func(arg);
    printf("%.3f\n", res);
  }

  dlclose(library);
  return 0;
}
