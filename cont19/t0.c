#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>

void EventCreate(int epoll_fd, int fd) {
  int flags = fcntl(fd, F_GETFL);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  struct epoll_event epoll_event;
  epoll_event.data.fd = fd;
  epoll_event.events = EPOLLIN | EPOLLHUP;  // если что-то записали или кто-то отключился
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_event);
}

const int NUM_OF_EVENTS = 4096;
size_t bytes_read = 0;
size_t files_left = 0;

void ProcessEvent(struct epoll_event* event) {
  char buffer[4096] = {};
  ssize_t cnt = 0;
  if ((cnt = read(event->data.fd, buffer, sizeof(buffer))) != 0) {
    bytes_read += cnt;
  } else {
    if (cnt == 0) {
      close(event->data.fd);
      --files_left;
    }
  }
}

extern size_t read_data_and_count(size_t N, int in[N]) {
  int epoll_fd = epoll_create(42);
  for (size_t i = 0; i < N; ++i) {
    EventCreate(epoll_fd, in[i]);
  }

  struct epoll_event events[NUM_OF_EVENTS];
  files_left = N;
  while (files_left > 0) {
    int events_read = epoll_wait(epoll_fd, events, NUM_OF_EVENTS,-1);
    for (size_t i = 0; i < events_read; ++i) {
      ProcessEvent(&events[i]);
    }
  }
  close(epoll_fd);
  return bytes_read;
}
