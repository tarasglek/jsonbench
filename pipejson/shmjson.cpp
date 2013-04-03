/*
A line-by-line multiplexer. This takes input on stdin, and feeds it to stdin of a user-requested number of processes
Unfortunately, it's memory-bound atm due to number of copies imposed by pipe

 */
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <utility>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include "unix_util.h"
#include "../json.h"
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/resource.h>

using namespace std;
// todo look at rust flatpipes

static long wrotesum = 0;
static long readsum = 0;
static size_t shmem_size = 0;
static size_t pipe_buffer_size = 0;

long timeval2ms(timeval &t) {
  return t.tv_sec * 1000 + t.tv_usec/1000;
}

void print_rusage() {
  rusage usage;
  if (0 != getrusage(RUSAGE_SELF, &usage)){
    err(1, "getrusage");
  }
  fprintf(stderr, "user CPU time:%lu;system CPU time:%lu ru_minflt:%ld ru_majflt:%ld\n",
          timeval2ms(usage.ru_utime),  timeval2ms(usage.ru_stime), usage.ru_minflt, usage.ru_majflt);
}
struct DataMarker {
  DataMarker(size_t begin=0, size_t length=0):begin(begin), length(length) {}
  unsigned int begin;
  unsigned int length;
};

struct FDBuffer {
  enum Status {
    READY,
    BLOCKED,
    INPUT_EOF
  };

  int fd;
  pid_t pid;
  void *shmem;
  size_t shmem_offset;
  // start_busy contains the start of window(in shmem) that's currently in queue to the child process
  vector<DataMarker> window;
  DataMarker *todo;
  FDBuffer(int fd = -1, pid_t pid = 0, void *shmem = NULL):
    fd(fd), pid(pid), shmem(shmem), shmem_offset(0), todo(NULL)
  {
  }
  
  Status write_outstanding() {
    if (!todo)
      return READY;
    int wrote = write(fd, todo, sizeof(*todo));
    if (wrote != sizeof(*todo)) {
      if (wrote != -1)
        err(1, "partial writes should never occur");
      if (errno == EAGAIN)
        return BLOCKED;
      else
        err(1, "unknown write() error");
    }
    todo = NULL;
    if (window.size() > pipe_buffer_size/sizeof(DataMarker)) {
      //window.erase(window.begin());
      //size_t begin = 
      // fprintf(stderr, "maintaining Window.count=%lu of %lubytes\n", 
      //      window.size(), window.back().begin + window.back().length - window.front().begin);
    }

    return READY;
  }

  // move data from line_buffer into fd
  // return -1 when EOF
  // return 0 when fd wont accept any more bytes
  // return 1 when ready to write more
  Status fwd(FILE *input) {
    if (write_outstanding() == BLOCKED)
      return BLOCKED;

    size_t line_buffer_size = shmem_size - shmem_offset;
    //fprintf(stderr, "line_buffer_size=%lu\n", line_buffer_size);

    char *dest = static_cast<char*>(shmem) + shmem_offset;
    // DANGER: getline expects to be able to reallocate dest if line wont fit..it better fit
    ssize_t fwd_size = getline(&dest, &line_buffer_size, input);

    // input EOF
    if (fwd_size == -1)
      return INPUT_EOF;

    // make sure what we send is \0-terminated
    /*if (dest[fwd_size-1] == '\n')
      dest[fwd_size-1] = 0;
      else*/
    // include null in transmitted buffer
    fwd_size++;

    DataMarker d(shmem_offset, fwd_size);
    //fprintf(stderr, "shmem_offset:%lu fwd_size:%lu\n", shmem_offset, fwd_size);
    //WARNING nothing actually ensures that window + getline don't overlap(other than ample room in shared mem)
    window.resize(1);
    window[0] = d;//.push_back(d);
    todo = &window.back();

    shmem_offset += fwd_size;
    if (shmem_offset + 1024*1024 >= shmem_size)
      shmem_offset = 0;

    readsum += fwd_size;
    return READY;
  }

  // not using a destructor cos copy constructors make a mess of fd-management
  void close() {
    ::close(fd);
    fd = -1;
  }
};
typedef vector<FDBuffer> fd_vector;

void
select_wait(fd_vector &sockets)
{
  fd_set wrds;
  struct timeval tv;
  int retval;

  /* Watch stdin (fd 0) to see when it has input. */
  FD_ZERO(&wrds);
  int maxfd = 0;
  for(fd_vector::iterator it = sockets.begin();it != sockets.end();it++) {
    if (it->fd == -1)
      continue;
    FD_SET(it->fd, &wrds);
    maxfd = max(it->fd, maxfd);
  }
  maxfd++;

  /* Wait up to five seconds. */
  tv.tv_sec = 5;
  tv.tv_usec = 0;

  retval = select(maxfd, NULL, &wrds, NULL, &tv);
  /* Don't rely on the value of tv now! */

  if (retval == -1)
    perror("select()");
  else if (retval) {
    //printf("time to write\n");
  }
}

int main(int argc, char **argv) {
  fd_vector workers;
  fd_vector::iterator pipe_iterator;
  if (argc < 3) {
    fprintf(stderr, "Usage; %s <process number> <cmd + args>", argv[0]);
    _exit(1);
  }
  int worker_count = atoi(argv[1]);
  for(int worker = 0;worker < worker_count;worker++) {
    // multiple of 2
    int pipes[2] = {0,0};
    if (pipe(pipes)) {
      perror("pipe");
      _exit(1);
    }

    int outfd = pipes[1];
    // set the buffer to include enough room for 256 markers
    if (fcntl(outfd, F_SETPIPE_SZ, 4096) == -1) 
      perror("fcntl");

    pipe_buffer_size = fcntl(outfd, F_GETPIPE_SZ, 0);
    fprintf(stderr, "Pipe size = %lu\n", pipe_buffer_size);

    void *shmem = establish_shm_segment(pipe_buffer_size/sizeof(DataMarker) * 128/4, &shmem_size);
    fprintf(stderr,"%p %lu\n", shmem, shmem_size);
    int pid = fork();
    if (pid == -1) {
      perror("fork");
      _exit(1);
    }


    if (pid == 0) {    /* Child reads from pipe */
      close(pipes[1]);          /* Close unused write end */
      nice(1);
#define STDIN 0
      if (dup2(pipes[0], STDIN) == -1) {
        perror("dup2");
        _exit(1);
      }
      close(pipes[0]);
      //prctl (PR_SET_NAME, "json", 0, 0, 0);
      //setaffinity(worker);
      
      DataMarker incoming_d;
      int r;
      {
      JSONBench json;
      while((r = read(STDIN, &incoming_d, sizeof(incoming_d)))) {
        if (r == -1)
          break;
        else if (r != sizeof(incoming_d)) {
          fprintf(stderr, "Receiver undeflow %d/%lu\n", r, sizeof(incoming_d));
          err(1, "read() give up");
        }
        //fprintf(stderr, "start:%lu, length:%lu\n", incoming_d.begin, incoming_d.length);
        char *buf = static_cast<char*>(shmem) + incoming_d.begin;
        //buf[incoming_d.length-1] = 0;
        //        fprintf(stderr, "end char=%d\n", buf[incoming_d.length-1]);
        //write(1, buf, incoming_d.length-1);
        //user CPU time:16877;system CPU time:3512 ru_minflt:3632733 ru_majflt:0
        //user CPU time:16753;system CPU time:3652 ru_minflt:3632733 ru_majflt:0
        //user CPU time:16753;system CPU time:3600 ru_minflt:3632733 ru_majflt:0

        char* ptr =static_cast<char*>(shmem)+(incoming_d.begin/4096 * 4096);
        /*  if (-1 == madvise(ptr, (incoming_d.length/4096+1)*4096, MADV_WILLNEED)) {
          perror("madvise");
          }*/
        json.parse(buf, incoming_d.length-1);
        //r+=strlen(buf);
      }
      }
      //execv(argv[2], argv+2);
      //fprintf(stderr, "Failed to launch subprocess: %s\n", argv[2]);
      print_rusage();
      _exit(1);
      // process is replaced with child
    }
    // parent
    close(pipes[0]);          /* Close unused read end */    

    int flags = fcntl(outfd, F_GETFL, 0);
    if (fcntl(outfd, F_SETFL, flags | O_NONBLOCK) == -1) 
      perror("fcntl");
    FDBuffer child(outfd, pid, shmem);
    workers.push_back(child);
  }
  //setaffinity(0);
  //nice(0);
  pipe_iterator = workers.begin();
  bool pipes_full = false;
  if (-1 == posix_fadvise (0,0,0, POSIX_FADV_NOREUSE | POSIX_FADV_SEQUENTIAL| POSIX_FADV_WILLNEED))
    err(1, "fadvise");
  while(true) {
    const int pipe_status = pipe_iterator->fwd(stdin);
    if (pipe_status == FDBuffer::INPUT_EOF)
      break;
    else if (pipe_status == FDBuffer::BLOCKED) {
      pipe_iterator++;
      if (pipe_iterator == workers.end()) {
        pipe_iterator = workers.begin();
        // require one full everything-is-blocked iteration before giving up and trying to select
        if (pipes_full) {
          select_wait(workers);
          //fprintf(stderr, "waiting\n");
        }
        pipes_full = true;
      }
    } else if (pipe_status == FDBuffer::READY) {
      pipes_full = false;
      continue;
    }
  }
  for(pipe_iterator = workers.begin();pipe_iterator != workers.end(); pipe_iterator++) {
    while(pipe_iterator->write_outstanding() == FDBuffer::BLOCKED)
      select_wait(workers);
    pipe_iterator->close();
  } 


  while (true) {
    pid_t done = wait(NULL);
    if (done == -1) {
      if (errno == ECHILD) break; // no more child processes
    }
  }
  fprintf(stderr, "read:%ld wrote:%ld diff:%d\n", readsum, wrotesum, (int)(readsum - wrotesum));
  print_rusage();
}
