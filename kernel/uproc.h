#define SYS_getprocs 25

struct uproc {
  int pid;
  int ppid;
  int state;
  uint64 sz;
  char name[16];
};