#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

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

// char* strtok(char *string, const char *delim);
// int lstat(const char *file_name, struct stat *buf);
// В случае, если pathname является символьной ссылкой, то возвращается
// информация о самой ссылке, а не о файле, на который она указывает.

// ssize_t readlink(const char* restrict pathname, char* restrict buf, size_t bufsiz);
// char* dirname(char* path);
// char* realpath(const char* restrict path, char* restrict resolved_path);
// int symlink(const char* target, const char* linkpath); - creates a symbolic link named linkpath which contains
// the string target.

int main() {
  char file_name[PATH_MAX];  // строка максимально возможной длины для названия файла
  struct stat st;
  while (fgets(file_name, PATH_MAX, stdin) != NULL) {
    *(file_name + strlen(file_name) - 1) = '\0';
    if (lstat(file_name, &st) == -1) {  // записываем инфу о переданном аргументе и проверяем, удачно ли это сделали
      continue;
    }
    if (S_ISLNK(st.st_mode)) {  // если это символьная ссылка
      char file[PATH_MAX];
      ssize_t size_of_read = readlink(file_name, file, sizeof(file));
      if (size_of_read != -1) {
        *(file + size_of_read) = '\0';
        char* path_not_including_curr_dir = dirname(file_name);
        char absolute_path[PATH_MAX];
        sprintf(absolute_path, "%s/%s", path_not_including_curr_dir, file);
        char resolved_absolute_path[PATH_MAX];
        realpath(absolute_path, resolved_absolute_path);
        printf("%s\n", resolved_absolute_path);
      }
      continue;
    }
    if (S_ISREG(st.st_mode)) {  // если регулярный файл
      char* cur_dir = basename(file_name);
      char symlink_name[PATH_MAX] ;
      strcat(symlink_name, "link_to_");
      strcat(symlink_name, cur_dir);
      symlink(file_name, symlink_name);
    }
  }
  return  0;
}