#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

char* path;
int client_fd;
int socket_fd;

void SIG_Handler() {
  close(socket_fd);
  shutdown(client_fd, SHUT_RDWR);
  close(client_fd);
  exit(0);
}

void server_answer(char* request, int client) {
  char* filename = strtok(request + 4, " ");
  char path_to_file[4096];
  memset(path_to_file, 0, sizeof(path_to_file));
  snprintf(path_to_file, sizeof(path_to_file), "%s/%s", path, filename);

  int file_exists = access(path_to_file, F_OK);
  int file_opens = access(path_to_file, R_OK);
  int file_executes = access(path_to_file, X_OK);

  if (file_exists == 0) {
    if (file_opens == 0) {
      if (file_executes == 0) {
        char answer[4096];
        memset(answer, 0, sizeof(answer));
        strcpy(answer, "HTTP/1.1 200 OK\r\n");
        write(client_fd, answer, strlen(answer));

        pid_t pid = fork();
        if (pid == 0) {
          dup2(client_fd, 1);
          close(client_fd);
          execlp(path_to_file, path_to_file, NULL);
          perror("exec error");
          exit(1);
        }
        waitpid(pid, NULL, 0);
      } else {
        struct stat file_info;
        lstat(path_to_file, &file_info);
        char answer[4096];
        memset(answer, 0, sizeof(answer));
        strcpy(answer, "HTTP/1.1 200 OK\r\n");
        write(client, answer, strlen(answer));

        memset(answer, 0, sizeof(answer));
        snprintf(answer, sizeof(answer), "Content-Length: %lu\r\n", file_info.st_size);
        write(client, answer, strlen(answer));
        write(client, "\r\n", 2);

        int fd = open(path_to_file, O_RDONLY);
        char contents[4096];
        memset(contents, 0, sizeof(contents));
        size_t read_size = 0;
        while ((read_size = read(fd, contents, sizeof(contents))) > 0) {
          write(client_fd, contents, read_size);
        }
        write(client_fd, "\r\n", 2);
        close(fd);
      }
    } else {
      char answer[4096];
      memset(answer, 0, sizeof(answer));
      strcpy(answer, "HTTP/1.1 403 Forbidden\r\n");
      write(client_fd, answer, strlen(answer));

      memset(answer, 0, sizeof(answer));
      snprintf(answer, sizeof(answer), "Content-Length: %d\r\n", 0);
      write(client_fd, answer, strlen(answer));
      write(client_fd, "\r\n", 2);
    }
  } else {
    char answer[4096];
    memset(answer, 0, sizeof(answer));
    strcpy(answer, "HTTP/1.1 404 Not Found\r\n");
    write(client_fd, answer, strlen(answer));
    write(client_fd, "\r\n", 2);
  }

}

int main(int argc, char* argv[]) {
  // Достаю аргументы
  uint32_t port = htons(strtol(argv[1], NULL, 10));
  path = argv[2];
  uint32_t address = inet_addr("127.0.0.1");

  // Создаю сокет
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = port;
  addr.sin_addr.s_addr = address;

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    perror("socket create error");
    exit(1);
  }

  // Создаю сервер
  if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1) {
    close(socket_fd);
    perror("bind error");
    exit(1);
  }

  // Жду запросов
  if (listen(socket_fd, SOMAXCONN) == -1) {
    close(socket_fd);
    perror("listen error");
    exit(1);
  }

  // Делаю обработчик сигналов
  struct sigaction sigint_handler;
  memset(&sigint_handler, 0, sizeof(sigint_handler));
  sigint_handler.sa_handler = SIG_Handler;
  sigint_handler.sa_flags = SA_SIGINFO;
  sigaction(SIGINT, &sigint_handler, NULL);
  sigaction(SIGTERM, &sigint_handler, NULL);

  // Считываю запрос
  char request[8192];
  char buffer[4096];
  while(1) {
    client_fd = accept(socket_fd, NULL, NULL);
    if (client_fd == -1) {
      close(client_fd);
      close(socket_fd);
      perror("accept error");
      exit(1);
    }
    memset(request, 0, 8192);
    size_t read_len = 0;
    while (1) {
      memset(buffer, 0, sizeof(buffer));
      read(client_fd, buffer, sizeof(buffer));
      strcpy(request + read_len, buffer);
      read_len += strlen(buffer);
      if (strcmp(request + strlen(request) - 4, "\r\n\r\n") == 0) {
        server_answer(request, client_fd);
        break;
      }
    }
    close(client_fd);
  }
}
