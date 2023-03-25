#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

int main(int argc, char** argv) {
  uint32_t address = inet_addr(argv[1]);
  uint16_t port = htons(strtol(argv[2], NULL, 10));

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = port;
  addr.sin_addr.s_addr = address;

  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    perror("socket create error");
    exit(1);
  }

  if (connect(socket_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1) {
    close(socket_fd);
    perror("connection error");
    exit(1);
  }

  int write_buffer = 0;
  int read_buffer = 0;
  while(scanf("%d", &write_buffer) != EOF) {
    if (write(socket_fd, &write_buffer, sizeof(int)) < 1 ||
        read(socket_fd, &read_buffer, sizeof(int)) < 1) {
      break;
    }
    printf("%d\n", read_buffer);
  }
  close(socket_fd);
  shutdown(socket_fd, SHUT_RDWR);
  return 0;
}
