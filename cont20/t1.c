#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>

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

size_t GetFileLength(char* filename) {
  struct stat file_info;
  stat(filename, &file_info);
  return file_info.st_size;
}

void SendRequest(char* addr, char* path, char* filename) {
  size_t file_length = GetFileLength(filename);

  int file_fd = open(filename, O_RDONLY);
  if (file_fd == -1) {
    perror("file open error");
    close(socket_fd);
    exit(1);
  }

  char request[4096] = {};
  size_t request_header_length = snprintf(request, sizeof(request),
           "POST %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Content-Type: multipart/form-data\r\n"
           "Connection: close\r\n"
           "Content-Length: %zu\r\n\r\n",
           path, addr, file_length);
  write(socket_fd, request, request_header_length);

  // sendfile отправит не более 2 147 479 552 байт, чего более чем достаточно для запроса,
  // поэтому цикл можно не делать
  if (sendfile(socket_fd, file_fd, NULL, file_length) == -1) {
    perror("sendfile error");
    close(file_fd);
    close(socket_fd);
    exit(1);
  }
  close(file_fd);
}

void ProcessResponse() {
  char response[4096] = {};
  ssize_t bytes_received = recvfrom(socket_fd, response, sizeof(response), 0, NULL, NULL);
  if (bytes_received == -1) {
    perror ("receive error");
    close(socket_fd);
    exit(1);
  }

  char* answer_pos = strstr(response, "\r\n\r\n") + strlen("\r\n\r\n");
  write(1, answer_pos, bytes_received - (answer_pos - response));
}

int main(int argc, char** argv) {
  char* server_name = argv[1];
  char* path_to_script = argv[2];
  char* filename = argv[3];
  // Получу адрес хоста
  struct addrinfo* address_info = GetAddrInfo(server_name);
  // Подключусь к серверу
  ConnectToServer(address_info);
  // Пошлю запрос
  SendRequest(server_name, path_to_script, filename);
  // Обработаю ответ
  ProcessResponse();
  // Отключусь от сервера
  shutdown(socket_fd, SHUT_RDWR);
  close(socket_fd);
  return 0;
}
