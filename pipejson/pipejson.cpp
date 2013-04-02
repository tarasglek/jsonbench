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

using namespace std;
// todo look at rust flatpipes

static long wrotesum = 0;
static long readsum = 0;
static size_t shmmem_size = 0;

struct FDBuffer {
  enum Status {
    READY,
    BLOCKED,
    INPUT_EOF
  };

  int fd;
  char *line_buffer;
  size_t line_buffer_size;
  ssize_t fwd_size;
  char *outbuf;
  size_t fwdsofar;
  pid_t pid;
  void *schmem;
  FDBuffer(int fd = -1, pid_t pid = 0, void *schmem = NULL):
    fd(fd), line_buffer(NULL), line_buffer_size(0), fwd_size(0), outbuf(NULL), fwdsofar(0), pid(pid), schmem(schmem)
  {
  }
  
  void malloc(size_t size) {
    line_buffer = (char*)::malloc(size);
    line_buffer_size = size;
  }

  Status write_outstanding() {
    if (!outbuf)
      return READY;
    const int todo_bytes = fwd_size - fwdsofar;
    int wrote = write(fd, outbuf, todo_bytes);

    if (wrote != -1) {
      wrotesum += wrote;
      fwdsofar += wrote;
    }
    if (wrote == todo_bytes) {
      outbuf = NULL;
      fwdsofar = fwd_size = 0;
      return READY;
    } 
    if (wrote == -1) {
      if (errno == EAGAIN) {
        //can't write anything else to socket
      } else {// IO ERROR
        perror("write");
        _exit(1);
      }

    } else if (wrote < todo_bytes) {
      outbuf += wrote;
    } else if (wrote > todo_bytes) {
      fprintf(stderr, "Impossible write()ed more than asked for\n");
      _exit(1);
    }
    return BLOCKED;
  }
  // move data from line_buffer into fd
  // return -1 when EOF
  // return 0 when fd wont accept any more bytes
  // return 1 when ready to write more
  Status fwd(FILE *input) {
    if (write_outstanding() == BLOCKED)
      return BLOCKED;
    
    fwd_size = getline(&line_buffer, &line_buffer_size, input);
    // input EOF
    if (fwd_size == -1)
      return INPUT_EOF;

    readsum += fwd_size;
    outbuf = line_buffer;
    fwdsofar = 0;
    return write_outstanding();
  }

  // not using a destructor cos copy constructors make a mess of fd-management
  void close() {
    ::close(fd);
    free(line_buffer);
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
    //100mb shmem
    void *shmmem = establish_shm_segment(25*1024, &shmmem_size);
    printf("%p %lu\n", shmmem, shmmem_size);
    int pid = fork();
    if (pid == -1) {
      perror("fork");
      _exit(1);
    }


    if (pid == 0) {    /* Child reads from pipe */
      close(pipes[1]);          /* Close unused write end */
#define STDIN 0
      if (dup2(pipes[0], STDIN) == -1) {
        perror("dup2");
        _exit(1);
      }
      close(pipes[0]);
      execv(argv[2], argv+2);
      fprintf(stderr, "Failed to launch subprocess: %s\n", argv[2]);
      _exit(1);
      // process is replaced with child
    }
    // parent
    close(pipes[0]);          /* Close unused read end */    

    int outfd = pipes[1];
    int flags = fcntl(outfd, F_GETFL, 0);
    if (fcntl(outfd, F_SETFL, flags | O_NONBLOCK) == -1) 
      perror("fcntl");
    // max buffer size on linux
    if (fcntl(outfd, F_SETPIPE_SZ, 1048576) == -1) 
      perror("fcntl");
    // max size if one bumps /proc/sys/fs/pipe-max-size
    fcntl(outfd, F_SETPIPE_SZ, 16777216);

    FDBuffer child(outfd, pid, shmmem);
    workers.push_back(child);
  }

  pipe_iterator = workers.begin();
  bool pipes_full = false;
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
}
