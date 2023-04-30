#define FUSE_USE_VERSION 30
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fuse3/fuse.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#define assertm(condition, error_info)    \
  do {                                    \
    if (!(condition)) {                   \
      perror(error_info);                 \
      exit(1);                            \
    }                                     \
  } while (0)

//////////////////////////////////////////////////////////
/// Structures.

typedef struct {
  const char* name;
  char* data;
  off64_t size;
} file_info_t;

typedef struct {
  char* src;
} fuse_options_t;

//////////////////////////////////////////////////////////
/// Globals

#define S_RXA 0555
#define S_RA 0444

static size_t dirs_count = 0;
static char** dir_names = NULL;

//////////////////////////////////////////////////////////
/// Helpers

int StrCmp(const void* a, const void* b) {
  const file_info_t* first = a;
  const file_info_t* second = b;
  return strcmp(first->name, second->name);
}

void CountNumOfDirs(char* dirs) {
  for (size_t i = 0; i < strlen(dirs); ++i) {
    if (dirs[i] == ':') {
      ++dirs_count;
    }
  }
  ++dirs_count;
}

void FindFile(const char* filename, char* dest, char* dir) {
  time_t mod_time = 0;
  for (size_t i = 0; i < dirs_count; ++i) {
    char full_path[PATH_MAX] = {};
    snprintf(full_path, PATH_MAX, "%s/%s", dir_names[i], filename);
    // Проверяем, существует ли файл
    if (access(full_path, F_OK) != 0) {
      continue;
    }
    // Если файл существуем, то получаем информацию о том, когда он модифицировался.
    struct stat file_info;
    stat(full_path, &file_info);
    if (dest[0] == '\0') {
      snprintf(dest, PATH_MAX, "%s", full_path);
      mod_time = file_info.st_mtim.tv_sec;
      if (dir != NULL) {
        snprintf(dir, PATH_MAX, "%s", dir_names[i]);
      }
    } else {
      // Если нашел, обновляю текущее название с помощью stat->st_mtim
      time_t cur_mod_time = file_info.st_mtim.tv_sec;
      if (mod_time < cur_mod_time) {
        snprintf(dest, PATH_MAX, "%s", full_path);
        mod_time = cur_mod_time;
        if (dir != NULL) {
          snprintf(dir, PATH_MAX, "%s", dir_names[i]);
        }
      }
    }
  }
}

void Cleanup() {
  for (size_t i = 0; i < dirs_count; ++i) {
    free((void*)dir_names[i]);
  }
  free(dir_names);
  dir_names = NULL;
  dirs_count = 0;
}

int FileExists(const char* path) {
  int exists = 0;
  for (size_t i = 0; i < dirs_count; ++i) {
    char path_with_dir[PATH_MAX] = {};
    snprintf(path_with_dir, PATH_MAX, "%s/%s", dir_names[i], path);
    if (access(path_with_dir, F_OK) == 0) {
      exists = 1;
      break;
    }
  }
  return exists;
}

//////////////////////////////////////////////////////////
/// Fuse operations.

int fuse_stat(const char* path, struct stat* st, struct fuse_file_info* fi);

int fuse_readdir(const char* path, void* out, fuse_fill_dir_t filler, off_t off,
                 struct fuse_file_info* fi, enum fuse_readdir_flags flags);

int fuse_open(const char* path, struct fuse_file_info* fi);

int fuse_read(const char* path, char* out, size_t size, off_t off,
              struct fuse_file_info* fi);

struct fuse_operations callbacks = {
    .getattr = fuse_stat,      /* Узнать инфу о файле              */
    .readdir = fuse_readdir,   /* Чтение директории                */
    .open = fuse_open,         /* Проверяем, можно ли открыть файл */
    .read = fuse_read,         /* Читаем файл                      */
};

int fuse_stat(const char* path,          // Путь к файлу
              struct stat* st,           // Структура с инфой о файле
              struct fuse_file_info* fi  // inode
) {
  // Получить инфу о текущей директории
  if (strcmp(path, "/") == 0) {
    st->st_mode = S_IFDIR | S_RXA;  /* Разрешения для файла. S_IFDIR - файл - директория, S_RXA = 0555 = 0 101 101 101 (только для чтения и исполнения) */
    st->st_nlink = 2;               /* Количество связей с файлом. Их 2, так как в любой директории есть как минимум 2 файла - . и ..                   */
    return 0;
  }

  // Проверим, существует ли файл в какой-нибудь директории
  if (FileExists(path) == 0) {
    return -ENOENT;
  }

  // Если файл - директория
  for (size_t i = 0; i < dirs_count; ++i) {
    char path_with_dir[PATH_MAX] = {};
    snprintf(path_with_dir, PATH_MAX, "%s/%s", dir_names[i], path);
    struct stat path_stat;
    ssize_t err = stat(path_with_dir, &path_stat);
    if (err == 0 && S_ISDIR(path_stat.st_mode)) {
      st->st_mode = S_IFDIR | S_RXA;
      st->st_nlink = path_stat.st_nlink;
      return 0;
    }
  }

  // Ну и регулярные файлы
  char file_path[PATH_MAX] = {};
  FindFile(path + 1, file_path, NULL);
  struct stat file_info;
  stat(file_path, &file_info);
  st->st_mode = S_IFREG | S_RA;         /* Разрешения для файла. S_IFREG - файл регулярный, S_RA = 0444 = 0 100 100 100 (только для чтения) */
  st->st_nlink = 1;                     /* Количество связей с файлом. Их 1 - вышестоящая директория                                        */
  st->st_size = file_info.st_size;      /* Размер файла в байтах                                                                            */
  // Если все ОК, то возвращаем 0
  return 0;
}

int fuse_readdir(const char* path,                // Путь к директории
                 void* out,                       // Буфер для записи ответа
                 fuse_fill_dir_t filler,          //
                 off_t off,                       // Сдвиг
                 struct fuse_file_info* fi,       // inode
                 enum fuse_readdir_flags flags    // Флаги
) {
  // Создаем 2 стандартных специальных файла
  filler(out, ".", NULL, 0, 0);
  filler(out, "..", NULL, 0, 0);

  // Добавляем все остальные файлы
  for (size_t i = 0; i < dirs_count; ++i) {
    char real_path[PATH_MAX] = {};
    snprintf(real_path, PATH_MAX, "%s/%s", dir_names[i], path);

    if (access(real_path, F_OK) != 0) {
      continue;
    }

    DIR* d;
    struct dirent* dir;
    d = opendir(real_path);
    while ((dir = readdir(d)) != NULL) {
      char full_path[PATH_MAX] = {};
      char file_dir[PATH_MAX] = {};
      char path_in_fs[PATH_MAX] = {};
      snprintf(path_in_fs, PATH_MAX, "%s/%s", path, dir->d_name);
      FindFile( path_in_fs, full_path, file_dir);
      if (strcmp(file_dir, dir_names[i]) != 0) {
        continue;
      }
      if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
        filler(out, dir->d_name, NULL, 0, 0);
      }
    }
    closedir(d);
  }
  // Если все ОК, то возвращаем 0
  return 0;
}

int fuse_open(const char* path,            // Путь к файлу
              struct fuse_file_info* fi    // inode
) {
  // Проверим, существует ли файл
  if (FileExists(path) == 0) {
    return -ENOENT;
  }

  // Получаем путь к файлу
  char file[PATH_MAX] = {};
  FindFile(path + 1, file, NULL);

  int cur_mode = fi->flags & O_ACCMODE;  // O_ACCMODE = 0003 = 0 000 000 011 (запись и исполнение)
  // Если файл закрыт на чтение
  if (cur_mode != O_RDONLY) {
    return -EACCES;
  }

  // Откроем файловый дескриптор для файла
  int file_fd = open(file, O_RDONLY);
  assertm(file_fd != -1, "file open failed");
  fi->fh = file_fd;

  // Если успех, то возвращаем 0
  return 0;
}

int fuse_read(const char* path,          // Путь к файлу
              char* out,                 //
              size_t size,               // Количество байт, которое нужно прочесть
              off_t off,                 // Сдвиг
              struct fuse_file_info* fi  //
) {
  // Если файл - директория
  for (size_t i = 0; i < dirs_count; ++i) {
    char path_with_dir[PATH_MAX] = {};
    snprintf(path_with_dir, PATH_MAX, "%s/%s", dir_names[i], path);
    struct stat path_stat;
    ssize_t err = stat(path_with_dir, &path_stat);
    if (err == 0 && S_ISDIR(path_stat.st_mode)) {
      close((int)fi->fh);
      return -EISDIR;
    }
  }

  // Найдем путь к файлу (мы уже проверили в fuse_open, что он существует)
  char file_path[PATH_MAX] = {};
  FindFile(path + 1, file_path, NULL);

  // Если сдвиг больше размера файла, то возвращаем 0
  struct stat file;
  ssize_t err = stat(file_path, &file);
  if (err == 0 && off >= file.st_size) {
    return 0;
  }
  // Если нужно прочесть байт больше, чем осталось до конца, то будем читать до конца
  if (off + size >= file.st_size) {
    size = file.st_size - off;
  }

  // Считаем из файла size байт
  int file_fd = (int)fi->fh;
  read(file_fd, out, size);
  close(file_fd);

  // Вернем количество прочитанных байт
  return (int)size;
}

int main(int argc, char** argv) {
  // Инициализация модифицируемого списка аргументов
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  // Специфические опции
  fuse_options_t fuse_options = {};
  // Массив структур для разбора опций
  struct fuse_opt opt_specs[] = {
      {"--src %s", offsetof(fuse_options_t, src), 0},
      {NULL      , 0                            , 0}};
  // Разбираем аргументы
  int err_ind = fuse_opt_parse(&args, &fuse_options, opt_specs, NULL);
  assertm(err_ind != -1, "fuse_opt_parse failed");

  // Что делать, если поступила команда src
  if (fuse_options.src) {
    char* token;
    // Считаем количество директорий
    CountNumOfDirs(argv[3]);
    dir_names = (char**)calloc(dirs_count, sizeof(char*));
    token = strtok(argv[3], ":");
    for (size_t i = 0; i < dirs_count; ++i) {
      dir_names[i] = (char*)calloc(strlen(realpath(token, NULL)), sizeof(char));
      snprintf(dir_names[i], strlen(realpath(token, NULL)) + 1, "%s", realpath(token, NULL));
      token = strtok(NULL, ":");
    }
  }
  // Запускаем демон
  int fuse_main_result = fuse_main(args.argc, args.argv,  // Аргументы
                                   &callbacks,            // Реализованные функции
                                   NULL                   // Указатель на какие-то пользовательские данные
                                   );
  Cleanup();
  return fuse_main_result;
}
