#include <arpa/inet.h>
#include <stdio.h>
#include <stdint-gcc.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct dns_header {
  unsigned id: 16;
  unsigned flags: 16;
  unsigned q_count: 16;
  unsigned ans_count: 16;
  unsigned auth_count: 16;
  unsigned add_count: 16;
} dns_header_t;

void GeneratePacket(char* packet) {
  dns_header_t* dns_header = (dns_header_t*)packet;
  *dns_header = (dns_header_t) {
    .id = (unsigned short) htons(getpid()), // A value that specifies the unique DNS message identifier.
    .flags = 0x0001,  // 1 на бите, отвечающем за рекурсию, на остальных - 0 (уже после htons())
    .q_count = htons(1),  // 1 запрос
    .ans_count = 0,
    .auth_count = 0,
    .add_count = 0
  };
}

const size_t DNS_HEADER_SIZE = sizeof(dns_header_t);

void ConvertAddressToDnsFormat(char* old_hostname, char* new_hostname) {
  int lock = 0;
  strcat(old_hostname, ".");

  for(int i = 0 ; i < strlen(old_hostname); ++i) {
    if (old_hostname[i] == '.') {
      *new_hostname++ = i - lock;
      for(; lock < i; ++lock) {
        *new_hostname++ = old_hostname[lock];
      }
      ++lock;
    }
  }
  *new_hostname++ = '\0';
}

typedef struct request {
  unsigned short type;
  unsigned short class;
} request_t;

size_t REQ_SIZE = 0;

void PrintAddr(char* src) {
  int ans_count = ntohs(((dns_header_t*)src)->ans_count);
  char* data = (char*)((dns_header_t*)&src[REQ_SIZE]);
  int shift = 0;

  for (int i = 0; i < ans_count; ++i) {
    shift += 10;
    int size_of_addr = ntohs(*(uint16_t*)(data + shift));
    shift += 2;
    shift += size_of_addr;
  }
  shift -= 4;
  printf("%hhu.%hhu.%hhu.%hhu\n", data[shift], data[shift + 1], data[shift + 2], data[shift + 3]);
  fflush(stdout);
}

int main() {
  int64_t dns_address = inet_addr("8.8.8.8");  // Адрес DNS-сервера

  int socket_fd = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
  if (socket_fd == -1) {
    perror("socket error");
    exit(1);
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = dns_address;
  addr.sin_port = htons(53);  // Порт DNS-сервера

  char packet[4096] = {};
  char hostname[4096] = {};
  while (scanf("%s", hostname) != EOF) {
    GeneratePacket(packet);
    char* name = &packet[sizeof(dns_header_t)];
    ConvertAddressToDnsFormat(hostname, name);
    request_t* request_info = (request_t*)&packet[sizeof(dns_header_t) + strlen(name) + 1];
    request_info->type = htons(1);  // IPv4
    request_info->class = htons(1);  // IN
    REQ_SIZE = sizeof(dns_header_t) + strlen(name) + 1 + sizeof(request_t);

    if (sendto(socket_fd,
               packet, sizeof(dns_header_t) + strlen(name) + 1 + sizeof(request_t),
               0,
               (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
      perror("sendto error");
      close(socket_fd);
      exit(1);
    }

    if (recvfrom(socket_fd,
                 packet, sizeof(packet),
                 0, NULL, NULL) == -1) {
      perror("recvfrom error");
      close(socket_fd);
      exit(1);
    }

    PrintAddr(packet);
  }
  close(socket_fd);
  return 0;
}
