#include <rapidjson/document.h>         // rapidjson's DOM-style API

#include <stdio.h>

#include <sys/time.h>


using namespace rapidjson;
using namespace std;

int main(int argc, char** argv) {
  
  FILE* input = stdin;
  if (argc == 2)
    input = fopen(argv[1], "r");

  struct timeval start, stop;
  gettimeofday(&start, NULL);
  Document document;	// Default template parameter uses UTF8 and MemoryPoolAllocator.
  long bytes = 0;
  long buf_size = 0;

  size_t line_buf_size = 1024;
  char *line_buf = (char*)malloc(buf_size);
  while(true) {
    ssize_t input_size = getline(&line_buf, &line_buf_size, input);
    if (input_size == -1)
      break;
      
    bytes += input_size;

    if (document.ParseInsitu<0>(line_buf).HasParseError())
      {
        // report to the user the failure and their locations in the document.
        fprintf(stderr, "Failed to parse json\n");
        return 0;
      }
 
  }
  free(line_buf);
  gettimeofday(&stop, NULL);
  time_t ms = (stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec)/1000;
  
  if (ms)
    printf("%ld MB/s %ld bytes in %ld seconds\n",  bytes*1000/1024/1024/ms, bytes, ms/1000);

}
