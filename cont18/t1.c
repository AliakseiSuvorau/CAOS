#include <arpa/inet.h>
#include <stdio.h>
#include <stdint-gcc.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

////////////////////////////////////////////////////////////////////////////////
///                             PING UTILITY (UDP)                           ///
////////////////////////////////////////////////////////////////////////////////

int socket_fd;
size_t got = 0;
const size_t IPV4_HEADER_SIZE = 20;
const size_t ICMP_DATA_SIZE = 48;
const size_t ICMP_HEADER_SIZE = 8;

typedef struct icmp_header {
  uint8_t icmp_type;
  uint8_t icmp_code;
  uint16_t check_sum;
  uint16_t icmp_id;
  uint16_t icmp_seq;
} icmp_header_t;

void SIGALRM_Handler() {
  char answer[4096] = {};
  snprintf(answer, sizeof(answer), "%lu\n", got);
  write(1, answer, strlen(answer));
  close(socket_fd);
  exit(0);
}

uint16_t CheckSum(void* packet_ptr, int len) {
  uint16_t* packet = packet_ptr;
  uint32_t sum = 0;
  for (; 1 < len; len -= 2) {
    sum += *packet++;
  }
  if (len == 1) {
    sum += *(uint8_t*)packet;
  }
  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  uint16_t res = ~sum;
  return res;
}

void GeneratePacket(char* packet) {
  icmp_header_t* packet_header = (icmp_header_t*)packet;
  *packet_header = (icmp_header_t) {
      // type=8, code=0 - эхо-запрос
      .icmp_type = 8,
      .icmp_code = 0,
      // пока ставим 0, позже заполним
      .check_sum = 0,
      // сюда любые числа, чтобы потом распознать отклик
      .icmp_id = 0,
      .icmp_seq = 0
  };

  packet_header->check_sum = CheckSum(packet, ICMP_DATA_SIZE + ICMP_HEADER_SIZE);
}

int ResponseHandler(char* response) {
  // эхо-отклик - type=0, code=0
  if (((icmp_header_t*)(response + IPV4_HEADER_SIZE))->icmp_type == 0 &&
      ((icmp_header_t*)(response + IPV4_HEADER_SIZE))->icmp_code == 0) {
    return 1;
  }
  return 0;
}

int main(int argc, char** argv) {
  int64_t address = inet_addr(argv[1]);
  int64_t timeout = strtol(argv[2], NULL, 10);
  int64_t interval = strtol(argv[3], NULL, 10);

  struct sigaction sig_handler;
  memset(&sig_handler, 0, sizeof(sig_handler));
  sig_handler.sa_flags = SA_RESTART;
  sig_handler.sa_handler = SIGALRM_Handler;
  if (sigaction(SIGALRM, &sig_handler, NULL) == -1) {
    perror("sigaction error");
    exit(1);
  }
  alarm(timeout);

  socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
  if (socket_fd == -1) {
    perror("socket error");
    exit(1);
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = address;

  char packet[ICMP_DATA_SIZE + ICMP_HEADER_SIZE];
  memset(&packet, 0, sizeof(packet));
  while(1) {
    GeneratePacket(packet);
    if (sendto(socket_fd, packet, sizeof(packet), 0,
           (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
      perror("sending error");
    }

    recvfrom(socket_fd, packet, sizeof(packet), 0, NULL, NULL);
    if (ResponseHandler(packet) == 1) {
      ++got;
    }

    usleep(interval);
  }
}
