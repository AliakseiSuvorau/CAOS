#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

int main() {
  char* bytes = calloc(2048, sizeof(char));
  size_t i = 0;
  while (scanf("%c", &bytes[i]) != EOF) {
    ++i;
  }
  unsigned char hash_arr[SHA512_DIGEST_LENGTH];
  // Кодируем входные данные
  SHA512((const unsigned char*)bytes, strlen(bytes), hash_arr);
  printf("0x");
  for (int j = 0; j < SHA512_DIGEST_LENGTH; ++j) {
    printf("%02x", hash_arr[j]);
  }
  free(bytes);
  return 0;
}
