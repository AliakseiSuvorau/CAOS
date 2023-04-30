#define FUSE_USE_VERSION 30
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fuse3/fuse.h>
#include <sys/mman.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>

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
  const char *name;
  char *data;
  off64_t size;
} file_info_t;

typedef struct {
  char *src;
} fuse_options_t;

//////////////////////////////////////////////////////////
/// Globals

#define S_RXA 0555
#define S_RA 0444

static size_t file_count = 0;
static file_info_t *files = NULL;

//////////////////////////////////////////////////////////
/// Helpers

int StrCmp(const void *a, const void *b) {
  const file_info_t *first = a;
  const file_info_t *second = b;
  return strcmp(first->name, second->name);
}

off_t GetFileSize(int fd) {
  struct stat st;
  int fstat_result = fstat(fd, &st);
  assertm(fstat_result != -1, "GetFileSize: fstat failed");

  return st.st_size;
}

file_info_t *FindFile(const char *filename) {
  const file_info_t temp = {
      .name = filename         /* Установим для файла переданное имя */
  };
  // Ищем среди files filecount файлов с именами, записанными ранее в temp при помощи функции StrCmp
  return bsearch(&temp, files, file_count, sizeof(file_info_t), StrCmp);
}

void Cleanup() {
  for (size_t i = 0; i < file_count; ++i) {
    free((void*)files[i].name);
  }
  free(files);
  files = NULL;
  file_count = 0;
}

//////////////////////////////////////////////////////////
/// Fuse operations.

int fuse_stat(const char *path, struct stat *st, struct fuse_file_info *fi);

int fuse_readdir(const char *path, void *out, fuse_fill_dir_t filler, off_t off,
                 struct fuse_file_info *fi, enum fuse_readdir_flags flags);

int fuse_open(const char *path, struct fuse_file_info *fi);

int fuse_read(const char *path, char *out, size_t size, off_t off,
              struct fuse_file_info *fi);

struct fuse_operations callbacks = {
    .getattr = fuse_stat,      /* Узнать инфу о файле              */
    .readdir = fuse_readdir,   /* Чтение директории                */
    .open = fuse_open,         /* Проверяем, можно ли открыть файл */
    .read = fuse_read,         /* Читаем файл                      */
};

int fuse_stat(const char *path,          // Путь к файлу
              struct stat *st,           // Структура с инфой о файле
              struct fuse_file_info *fi  // inode
              ) {
  // Получить инфу о текущей директории
  if (strcmp(path, "/") == 0) {
    st->st_mode = S_IFDIR | S_RXA;  /* Разрешения для файла. S_IFDIR - файл - директория, S_RXA = 0555 = 0 101 101 101 (только для чтения и исполнения) */
    st->st_nlink = 2;               /* Количество связей с файлом. Их 2, так как в любой директории есть как минимум 2 файла - . и ..                   */
    return 0;
  }
  // Получим инфу о файле (сдвинем указатель пути на 1, чтобы не учитывать /)
  file_info_t *file = FindFile(path + 1);
  // Если bsearch вернула NULL, то файл не был найден
  if (file == NULL) {
    return -ENOENT;
  }
  // Иначе, файл был найден и в file записан указатель на него
  st->st_mode = S_IFREG | S_RA;  /* Разрешения для файла. S_IFREG - файл регулярный, S_RA = 0444 = 0 100 100 100 (только для чтения) */
  st->st_nlink = 1;              /* Количество связей с файлом. Их 1 - вышестоящая директория                                        */
  st->st_size = file->size;      /* Размер файла в байтах                                                                            */
  // Если все ОК, то возвращаем 0
  return 0;
}

int fuse_readdir(const char *path,                // Путь к файлу
                 void *out,                       // Буфер для записи ответа
                 fuse_fill_dir_t filler,          //
                 off_t off,                       // Сдвиг
                 struct fuse_file_info *fi,       // inode
                 enum fuse_readdir_flags flags    // Флаги
                 ) {
  // Если путь содержит директории, то выходим
  if (strcmp(path, "/") != 0) {
    return -ENOENT;
  }
  // Создаем 2 стандартных специальных файла
  filler(out, ".", NULL, 0, 0);
  filler(out, "..", NULL, 0, 0);
  // Добавляем все остальные файлы
  for (size_t i = 0; i < file_count; ++i) {
    filler(out, files[i].name, NULL, 0, 0);
  }
  // Если все ОК, то возвращаем 0
  return 0;
}

int fuse_open(const char *path,            // Путь к файлу
              struct fuse_file_info *fi    // inode
              ) {
  // Ищем файл и получаем путь к нему
  file_info_t *file = FindFile(path + 1);
  // Если файла нет
  if (file == NULL) {
    return -ENOENT;
  }

  int cur_mode = fi->flags & O_ACCMODE;  // O_ACCMODE = 0003 = 0 000 000 011 (запись и исполнение)
  // Если файл закрыт на чтение
  if (cur_mode != O_RDONLY) {
    return -EACCES;
  }
  // Если успех, то возвращаем 0
  return 0;
}

int fuse_read(const char *path,          // Путь к файлу
              char *out,                 //
              size_t size,               // Количество байт, которое нужно прочесть
              off_t off,                 // Сдвиг
              struct fuse_file_info *fi  //
              ) {
  // Найдем путь к файлу (мы уже проверили в fuse_open, что он существует)
  file_info_t *file = FindFile(path + 1);
  // Если сдвиг больше размера файла, то возвращаем 0
  if (off >= file->size) {
    return 0;
  }
  // Если нужно прочесть байт больше, чем осталось до конца, то будем читать до конца
  if (off + size >= file->size) {
    size = file->size - off;
  }
  // Запишем в буфер out size байт из содержимого файла
  memcpy(out, file->data, size);
  // Вернем количество прочитанных байт
  return (int)size;
}

//////////////////////////////////////////////////////////
/// Get input.

void ParseFilesystem(char* data) {
  int err_ind;
  char* ptr = data;
  int read = 0;
  err_ind = sscanf(data, "%zu %n", &file_count, &read);
  assertm(err_ind != 0, "ParseFilesystem: no count file");
  // Если файлов нет, то выходим
  if (file_count == 0) {
    return;
  }
  // Иначе сдвигаем каретку на ту часть данных, где начинается описание файлов
  ptr += read;
  // Выделяем место под хранение инфы о файлах
  files = calloc(file_count, sizeof(file_info_t));
  assertm(files != NULL, "ParseFilesystem: calloc failed");
  for (size_t i = 0; i < file_count; ++i) {
    err_ind = sscanf(ptr, "%ms %zu %n", &files[i].name, &files[i].size, &read);
    assertm(err_ind != 0, "ParseFilesystem: no file info");
    ptr += read;
  }
  // Пустая строка
  char* file_data = strstr(data, "\n\n") + 2;
  assertm(file_data != NULL, "ParseFilesystem: no empty line");
  // Записываем в массив files указатели на начало данных файлов
  files[0].data = file_data;
  for (size_t i = 1; i < file_count; ++i) {
    files[i].data = files[i - 1].data + files[i - 1].size;
  }
  // Отсортируем в лексикографическом порядке
  qsort(files, file_count, sizeof(file_info_t), StrCmp);
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

  off_t file_size = 0;
  char* data = NULL;
  // Что делать, если поступила команда src
  if (fuse_options.src) {
    // Открываем файловую систему
    int filesystem_src_fd = open(fuse_options.src, O_RDONLY);
    assertm(filesystem_src_fd != -1, "fuse open failed");

    file_size = GetFileSize(filesystem_src_fd);
    // Отобразим в data все данные из образа файловой системы для более удобной работы с ними
    data = mmap(NULL, file_size, PROT_READ, MAP_SHARED, filesystem_src_fd, 0);
    assertm(data != MAP_FAILED, "mmap failed");
    assertm(close(filesystem_src_fd) != -1, "close src failed");
    // Достанем все данные из образа файловой системы
    ParseFilesystem(data);
  }
  // Запускаем демон
  int fuse_main_result = fuse_main(args.argc, args.argv,  // Аргументы
                                   &callbacks,            // Реализованные функции
                                   NULL                   // Указатель на какие-то пользовательские данные
                                   );
  Cleanup();
  munmap(data, file_size);
  return fuse_main_result;
}

/**
Формат образа файловой системы:
 [Количество файлов]
 [Название файла] [Размер файла]
 ...
 [Название файла] [Размер файла]

 [Данные файла]
 ...
 [Данные файла]
*/
