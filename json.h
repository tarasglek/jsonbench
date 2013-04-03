#ifndef JSON_H
#define JSON_H

#include <rapidjson/document.h>         // rapidjson's DOM-style API
#include <sys/time.h>
#include <stdio.h>

struct JSONBench {
  struct timeval start, stop;
rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> document;	// Default template parameter uses UTF8 and MemoryPoolAllocator.
  size_t bytes;
  JSONBench():bytes(0) {
    gettimeofday(&start, NULL);
  }

  void parse(char *input, ssize_t input_size) {
      
    bytes += input_size;
    
    if (document.ParseInsitu<0>(input).HasParseError())
      {
        // report to the user the failure and their locations in the document.
        fprintf(stderr, "Failed to parse json\n");
        return;
      }
  }

  ~JSONBench() {
    gettimeofday(&stop, NULL);
    time_t ms = (stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec)/1000;
    
    if (ms)
      printf("%ld MB/s %ld bytes in %ld seconds\n",  bytes*1000/1024/1024/ms, bytes, ms/1000);
  }
};
#endif
