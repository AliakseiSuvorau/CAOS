#include <stdio.h>
#include <stdint.h>
#include <windows.h>

int main(int argc, char **argv) {
  HANDLE in = CreateFileA(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
  if (in == INVALID_HANDLE_VALUE) {
    CloseHandle(in);
    return 1;
  }
  uint32_t next_pointer = 1;
  int value = 0;
  while (next_pointer != 0) {
    DWORD bytes_read = 0;
    ReadFile(in, &value, sizeof(value), &bytes_read, NULL);
    if (bytes_read == 0) {
      CloseHandle(in);
      return 0;
    }
    ReadFile(in, &(next_pointer), sizeof(next_pointer), &bytes_read, NULL);
    printf("%d ", value);
    SetFilePointer(in, next_pointer, NULL, FILE_BEGIN);
  }
  CloseHandle(in);
  return 0;
}
