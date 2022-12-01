#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

//  struct stat {
//    dev_t     st_dev;         /* ID of device containing file */
//    ino_t     st_ino;         /* Inode number */
//    mode_t    st_mode;        /* File type and mode */
//    nlink_t   st_nlink;       /* Number of hard links */
//    uid_t     st_uid;         /* User ID of owner */
//    gid_t     st_gid;         /* Group ID of owner */
//    dev_t     st_rdev;        /* Device ID (if special file) */
//    off_t     st_size;        /* Total size, in bytes */
//    blksize_t st_blksize;     /* Block size for filesystem I/O */
//    blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */
//
//    struct timespec st_atim;  /* Time of last access */
//    struct timespec st_mtim;  /* Time of last modification */
//    struct timespec st_ctim;  /* Time of last status change */
//  };

// char *strtok(char *string, const char *delim);
// int lstat(const char *file_name, struct stat *buf);
// int fstat(int filedes, struct stat *buf);

int main() {
  char file_name[PATH_MAX];  // строка максимально возможной длины для названия файла
  struct stat st;
  while (fgets(file_name, PATH_MAX, stdin) != NULL) {
    strtok(file_name, "\n");  // обрезаем строку до \n
    if (access(file_name, X_OK) != 0) {
      continue;
    }
    int fd = open(file_name, O_RDONLY);
    // далее идут корректные файлы, которые отмечены в системе как исполняемые (возможно, они пустые)
    // выводим, если !# нет и ELF нет, либо если шабанг указывает на не корректный интерпретатор
    if (lseek(fd, 0, SEEK_END) < 2) {  // т.е. не содержит ни ELF, ни шабанга
      puts(file_name);
      close(fd);
      continue;
    }
    lseek(fd, 0, SEEK_SET);  // сдвигаем каретку в начало
    char first_symbol = 0, second_symbol = 0;
    read(fd, &first_symbol, sizeof(char));
    read(fd, &second_symbol, sizeof(char));
    if (first_symbol == '#' && second_symbol == '!') {  // если есть начало шабанга, то проверим, корректный ли в нем указан путь
      // нам надо проверить, является ли путь, который идет после #!, корректным, то есть ведет ли он к корректному исполняемому файлу
      // выделим путь
      lseek(fd, 2 * sizeof(char), SEEK_SET);
      char path[PATH_MAX];
      read(fd, path, sizeof(path));
      strtok(path, "\n");  // обрезаем строку до \n
      if (access(path, X_OK) == 0) {  // корректный путь
        close(fd);
        continue;
      } else {
        puts(file_name);
        close(fd);
        continue;
      }
    } else {
      lseek(fd, 2 * sizeof(char), SEEK_SET);  // перемещаем каретку на 3-й символ
      char third_symbol = 0, fourth_symbol = 0;
      read(fd, &third_symbol, sizeof(char));
      read(fd, &fourth_symbol, sizeof(char));
      if (!(first_symbol == 0x7f && second_symbol == 'E' && third_symbol == 'L' && fourth_symbol == 'F')) {  // если это не ELF-файл
        puts(file_name);
        close(fd);
        continue;
      }
    }
    close(fd);
  }
  return 0;
}