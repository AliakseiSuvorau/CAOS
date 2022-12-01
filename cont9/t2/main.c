extern void normalize_path(char* path) {
  char* read = path;
  char* write = path;
  char* last_slash = path;
  while (*read != '\0') {
    if (*read == '.') {
      if (*(read + 1) == '.' && *(read + 2) == '/') {  // если нашли ../
        if (read != last_slash) {  // если он не в самом начале
          write = last_slash + 1;
        }
        read += 3;
      } else {
        if (*(read + 1) == '/') {  // если нашли ./
          if (read != last_slash) {  // если он не в самом начале
            read += 2;
          }
        }
      }
    } else {
      if (*read == '/' && *(read + 1) == '/') {  // если идет несколько слешей подряд
        ++read;
      } else {
        if (*read == '/' && *(read + 1) != '.') {  // если идет только один слеш и после него нет точки (то есть не будет /../)
          last_slash = read;
        }
        // любые символы
        *write = *read;
        ++read;
        ++write;
      }
    }
  }
  *write = '\0';
}
