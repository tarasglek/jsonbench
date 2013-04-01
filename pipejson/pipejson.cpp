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

using namespace std;
// todo look at rust flatpipes
void
select_wait(int fd)
{
  fd_set wrds;
  struct timeval tv;
  int retval;

  /* Watch stdin (fd 0) to see when it has input. */
  FD_ZERO(&wrds);
  FD_SET(fd, &wrds);

  /* Wait up to five seconds. */
  tv.tv_sec = 1;
  tv.tv_usec = 0;

  retval = select(fd+1, NULL, &wrds, NULL, &tv);
  /* Don't rely on the value of tv now! */

  if (retval == -1)
    perror("select()");
  else if (retval) {
    // printf("time to write\n");
  }else
    printf("No data within five seconds.\n");
}


int main(int argc, char **argv) {
  vector<int> workers;
  for(int worker = 0;worker < 3;worker++) {
    // multiple of 2
    int pipes[2];
    if (pipe(pipes)) {
      perror("pipe");
      _exit(1);
    }

    int cpid = fork();
    if (cpid == -1) {
      perror("fork");
      _exit(1);
    }


    if (cpid == 0) {    /* Child reads from pipe */
      close(pipes[1]);          /* Close unused write end */
#define STDIN 0
      if (dup2(pipes[0], STDIN) == -1) {
        perror("dup2");
        _exit(1);
      }
      close(pipes[0]);
      execv(argv[1], argv+1);
      perror("execv");
      // process is replaced with child
    }
    // parent
    close(pipes[0]);          /* Close unused read end */    
    workers.push_back(pipes[1]);
  }
  {            /* Parent writes argv[1] to pipe */
    long wrotesum = 0;
    long readsum = 0;
    
    int outfd = workers[0];
    int flags = fcntl(outfd, F_GETFL, 0);
    fcntl(outfd, F_SETFL, flags | O_NONBLOCK);


    size_t buf_size = 1024;
    char *line_buf = (char*)malloc(buf_size);
    while(true) {
      ssize_t input_size = getline(&line_buf, &buf_size, stdin);
      const ssize_t total_input_size = input_size;
      if (input_size == -1)
        break;
      readsum += input_size;
      int wrotesofar = 0;
      char *outbuf = line_buf;
      while(wrotesofar != total_input_size) {
        int wrote = write(outfd, outbuf, input_size);
        if (wrote != -1) {
          wrotesum += wrote;
          wrotesofar += wrote;
        }
        if (wrote != input_size) {
          //printf("wrote %d/%ld\n", wrote, input_size);
          if (wrote == -1) {
            if (errno != EAGAIN)
              perror("write");
          }else if (wrote < input_size) {
            outbuf += wrote;
            input_size -= wrote;
          }
          select_wait(outfd);
        }
      }
      if (readsum != wrotesum)
        printf("max line buffer size:%lu read:%ld wrote:%ld diff:%d\n", buf_size, readsum, wrotesum, (int)(readsum - wrotesum));
    }
    free(line_buf);
    printf("line size:%lu read:%ld wrote:%ld\n", buf_size, readsum, wrotesum);
    for(vector<int>::iterator it = workers.begin();it != workers.end(); it++) {
      close(*it);          /* Reader will see EOF */
    } 
    wait(NULL);                /* Wait for child */
    _exit(0);
  }

}
