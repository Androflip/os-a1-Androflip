#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

int
match(char *pattern, char *name)
{
  if (*pattern == '\0')
    return *name == '\0';

  if (*pattern == '*') {
    pattern++;
    if (match(pattern, name))
      return 1;
    if (*name != '\0')
      return match(pattern - 1, name + 1);
    return 0;
  }

  if (*pattern == *name)
    return match(pattern + 1, name + 1);

  return 0;
}

void
find(char *path, char *pattern, int type)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, O_RDONLY)) < 0)
    return;

  if (fstat(fd, &st) < 0) {
    close(fd);
    return;
  }

  if (st.type != T_DIR) {
    close(fd);
    return;
  }

  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
    close(fd);
    return;
  }

  strcpy(buf, path);
  p = buf + strlen(buf);
  *p++ = '/';

  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;

    char name[DIRSIZ + 1];
    memmove(name, de.name, DIRSIZ);
    name[DIRSIZ] = '\0';

    memmove(p, name, DIRSIZ);
    p[DIRSIZ] = '\0';

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
      continue;

    if (stat(buf, &st) < 0)
      continue;

    if (match(pattern, name) && (type == 0 || type == st.type ||
                                 (type == T_FILE && st.type == T_DEVICE)))
      printf("%s\n", buf);

    if (st.type == T_DIR)
      find(buf, pattern, type);
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  int type = 0;
  int fd;
  struct stat st;

  if (argc != 3 && argc != 5) {
    fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
    exit(1);
  }

  if (argc == 5) {
    if (strcmp(argv[3], "-type") != 0) {
      fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
      exit(1);
    }

    if (strcmp(argv[4], "f") == 0)
      type = T_FILE;
    else if (strcmp(argv[4], "d") == 0)
      type = T_DIR;
    else {
      fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
      exit(1);
    }
  }

  fd = open(argv[1], O_RDONLY);
  if (fd < 0 || fstat(fd, &st) < 0 || st.type != T_DIR) {
    fprintf(2, "find: cannot open %s\n", argv[1]);
    if (fd >= 0)
      close(fd);
    exit(1);
  }

  close(fd);
  find(argv[1], argv[2], type);
  exit(0);
}