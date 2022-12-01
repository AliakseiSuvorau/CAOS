#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

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

int main() {
  char file_name[PATH_MAX];  // строка максимально возможной длины для названия файла
  struct stat st;
  uint64_t result = 0;  // размер всех файлов
  while (fgets(file_name, PATH_MAX, stdin) != NULL) {
    strtok(file_name, "\n");  // разбиваем строку на подстроки с помощью \n
    if (lstat(file_name, &st) != -1  /*если все хорошо и это корректный путь*/
        && S_ISREG(st.st_mode)) {  /*проверяет, является ли файл регулярным*/
      result += st.st_size;
    }
  }
  printf("%lu\n", result);
}