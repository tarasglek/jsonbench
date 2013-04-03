#ifndef UNIX_UTIL_H
#define UNIX_UTIL_H
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <stdio.h>
#include <unistd.h>
#include <sched.h>

void err(int code, char const *str){
  perror(str);
  _exit(code);
}

#define PAGE_SIZE 4096

void *
establish_shm_segment(int nr_pages, size_t *outSize)
{
  int fd;
  void *addr;
  const size_t size = PAGE_SIZE * nr_pages;
  fd = shm_open("/memflag_lat", O_RDWR|O_CREAT|O_EXCL, 0600);
  if (fd < 0)
    err(1, "shm_open(\"/memflag_lat\")");
  shm_unlink("/memflag_lat");
  if (ftruncate(fd, size) < 0)
    err(1, "ftruncate() shared memory segment");
  addr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED,
	      fd, 0);
  if (addr == MAP_FAILED)
    err(1, "mapping shared memory segment");

  close(fd);
  *outSize = size;
  return addr;
}
void
setaffinity(int cpunum)
{
  cpu_set_t *mask;
  size_t size;
  int i;
  int nrcpus = 160;
  pid_t pid;
  mask = CPU_ALLOC(nrcpus);
  size = CPU_ALLOC_SIZE(nrcpus);
  CPU_ZERO_S(size, mask);
  CPU_SET_S(cpunum, size, mask);
  pid = getpid();
  i = sched_setaffinity(pid, size, mask);
  if (i == -1)
    err(1, "sched_setaffinity");
  CPU_FREE(mask);
}

#endif
