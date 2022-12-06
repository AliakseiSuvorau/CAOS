#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

// void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
// The starting address for the new mapping is specified in addr.
// If addr is NULL, then the kernel chooses the (page-aligned) address at which to create the mapping
// The length argument specifies the length of the mapping
// The prot argument describes the desired memory protection of the mapping
// The flags argument determines whether updates to the mapping are visible to other processes mapping the same region, and whether updates are carried through to the underlying file.

// void* memmem(const void* haystack, size_t haystacklen, const void* needle, size_t needlelen);
// The memmem() function finds the start of the first occurrence of the substring needle of length needlelen in the memory area haystack of length haystacklen.

// int munmap(void *addr, size_t length);
// The munmap() function removes the mappings for pages in the range [addr, addr + len).

int main(int argc, char** argv) {
  int filename = open(argv[1], O_RDONLY);
  struct stat st;
  fstat(filename, &st);
  unsigned long size = st.st_size;
  char* contents = mmap(NULL, size, PROT_READ, MAP_PRIVATE, filename, 0);
  if (contents != MAP_FAILED) {
    char* entry = contents;
    while ((entry = memmem(entry, size, argv[2], strlen (argv[2]))) != NULL) {
      entry = memmem(entry, size, argv[2], strlen (argv[2]));
      unsigned long long diff = entry - contents;
      size = st.st_size - diff - 1;
      printf("%llu ", diff);
      ++entry;
    }
    munmap(contents, st.st_size);
  }
  close(filename);
  return 0;
}
