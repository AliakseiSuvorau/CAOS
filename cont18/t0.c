#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv) {
  int64_t port_number = strtol(argv[1], NULL, 10);

  int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_fd == -1) {
    perror("socket error");
    exit(1);
  }

  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_addr = inet_addr("127.0.0.1"),
      .sin_port = htons(port_number)
  };

  int send_num = 0;
  int recv_num = 0;
  while (scanf("%d", &send_num) != EOF) {
    sendto(socket_fd, &send_num, sizeof(send_num), 0, (const struct sockaddr*)&addr, sizeof(addr));
    recvfrom(socket_fd, &recv_num, sizeof(recv_num), 0, NULL, NULL);
    printf("%d\n", recv_num);
  }
  close(socket_fd);
  return 0;
}
