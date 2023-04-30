#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

int socket_fd;

struct addrinfo* GetAddrInfo(char* hostname) {
  struct addrinfo* addr = NULL;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family =  AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  struct addrinfo* res = NULL;
  if (getaddrinfo(hostname, "http", &hints, &res) != 0) {
    perror ("getaddrinfo error");
    exit(1);
  }
  return res;
}

void ConnectToServer(struct addrinfo* address_info) {
  socket_fd = socket(address_info->ai_family,
                     address_info->ai_socktype,
                     address_info->ai_protocol);
  if (socket_fd == -1) {
    perror ("socket create error");
    exit(1);
  }

  if (connect(socket_fd, address_info->ai_addr, address_info->ai_addrlen) == -1) {
    perror ("connect error");
    close(socket_fd);
    exit(1);
  }
}

void SendRequest(char* addr, char* path) {
  char request[4096] = {};
  snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", path, addr);
  write(socket_fd, request, strlen(request));
}

void ProcessResponse() {
  char response[4096] = {};
  ssize_t bytes_received = recvfrom(socket_fd, response, sizeof(response), 0, NULL, NULL);
  if (bytes_received == -1) {
    perror ("receive error");
    close(socket_fd);
    exit(1);
  }
  char* length_pos = strstr(response, "Content-Length: ") + strlen("Content-Length: ");
  size_t content_length = 0;
  sscanf(length_pos, "%zu", &content_length);
  char* answer_pos = strstr(response, "\r\n\r\n") + strlen("\r\n\r\n");
  size_t bytes_written = write(1, answer_pos, bytes_received - (answer_pos - response));
  content_length -= bytes_written;

  while (content_length > 0) {
    bytes_received = recvfrom(socket_fd, response, sizeof(response), 0, NULL, 0);
    if (bytes_received == -1) {
      perror ("receive error");
      close(socket_fd);
      exit(1);
    }
    bytes_written = write(1, response, bytes_received);
    content_length -= bytes_written;
  }
}

int main(int argc, char** argv) {
  char* domain_server_name = argv[1];
  char* filepath = argv[2];
  // Получу адрес хоста
  struct addrinfo* address_info = GetAddrInfo(domain_server_name);
  // Подключусь к серверу
  ConnectToServer(address_info);
  // Пошлю запрос
  SendRequest(domain_server_name, filepath);
  // Обработаю ответ
  ProcessResponse();
  // Отключусь от сервера
  shutdown(socket_fd, SHUT_RDWR);
  close(socket_fd);
  return 0;
}
