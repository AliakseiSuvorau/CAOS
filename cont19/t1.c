#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>

int client_fd;
int socket_fd;
int epoll_fd;

void ChangeRegister(char* src, char* dest) {
  for (int i = 0; i < strlen(src); ++i) {
    if (src[i] >= 'a' && src[i] <= 'z') {
      dest[i] = src[i] - 32;
      continue;
    }
    dest[i] = src[i];
  }
}

void SIG_Handler() {
  close(socket_fd);
  shutdown(client_fd, SHUT_RDWR);
  close(client_fd);
  close(epoll_fd);
  exit(0);
}

void EventCreate(int fd) {
  int flags = fcntl(fd, F_GETFL);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  struct epoll_event epoll_event;
  epoll_event.data.fd = fd;
  epoll_event.events = EPOLLIN | EPOLLHUP;  // если что-то записали или кто-то отключился
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_event);
}

const int NUM_OF_EVENTS = 4096;
size_t files_left = 0;

void ProcessEvent(struct epoll_event* event) {
  if (event->data.fd == socket_fd) {
    client_fd = accept(socket_fd, NULL, NULL);
    if (client_fd == -1) {
      close(client_fd);
      close(socket_fd);
      perror("accept error");
      exit(1);
    }
    EventCreate(client_fd);
  } else {
    char buffer[4096] = {};
    ssize_t cnt = 0;
    if ((cnt = read(event->data.fd, buffer, sizeof(buffer))) != 0) {
      char answer[4096] = {};
      ChangeRegister(buffer, answer);
      write(client_fd, answer, strlen(answer));
      close(client_fd);  /////////////////////////////////////////////////////////////////////
    } else {
      if (cnt == 0) {
        close(event->data.fd);
        --files_left;
      }
    }
  }
}

int main(int argc, char* argv[]) {
  // Достаю аргумент
  uint32_t port = htons(strtol(argv[1], NULL, 10));
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

  // Делаю очередь запросов
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
  sigaction(SIGTERM, &sigint_handler, NULL);

  // Создаю очередь event'ов
  epoll_fd = epoll_create(42);
  EventCreate(socket_fd);
  struct epoll_event events[NUM_OF_EVENTS];

  while(1) {
    int events_read = epoll_wait(epoll_fd, events, NUM_OF_EVENTS,-1);
    for (size_t i = 0; i < events_read; ++i) {
      ProcessEvent(events);
    }
  }
}
