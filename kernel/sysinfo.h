#define SYS_sysinfo 24

struct sysinfo {
  uint64 freemem;
  uint64 nproc;
  uint64 nopenfile;
};