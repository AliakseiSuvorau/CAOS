#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint-gcc.h>

// void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
// The starting address for the new mapping is specified in addr.
// If addr is NULL, then the kernel chooses the (page-aligned) address at which to create the mapping
// The length argument specifies the length of the mapping
// The prot argument describes the desired memory protection of the mapping
// The flags argument determines whether updates to the mapping are visible to other processes mapping the same region, and whether updates are carried through to the underlying file.

// int munmap(void *addr, size_t length);
// The munmap() function removes the mappings for pages in the range [addr, addr + len).

int main(int argc, char **argv) {
  int in = open(argv[1], O_RDONLY);
  if (in == -1) {
    close(in);
    return 1;
  }
  struct stat st;
  fstat(in, &st);
  unsigned long size = st.st_size;
  if (size == 0) {
    close(in);
    return 0;
  }
  int* contents = mmap(NULL, size, PROT_READ, MAP_PRIVATE, in, 0);
  int value = contents[0];
  printf("%d ", value);
  uint32_t next_pointer = contents[1];
  while (next_pointer != 0) {
    value = contents[next_pointer / sizeof(int)];
    printf("%d ", value);
    next_pointer = contents[next_pointer / sizeof(int) + 1];
  }
  munmap(contents, st.st_size);
  close(in);
  return 0;
}
