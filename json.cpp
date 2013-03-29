#include <rapidjson/document.h>         // rapidjson's DOM-style API

#include <fstream>
#include <sys/time.h>
#include <iostream>

using namespace rapidjson;
using namespace std;

int main(int argc, char** argv) {
  std::string line;

  std::ifstream input( argv[1] );
  struct timeval start, stop;
  int i = gettimeofday(&start, NULL);
Document document;	// Default template parameter uses UTF8 and MemoryPoolAllocator.
  long bytes = 0;
  char *buf = NULL;
long buf_size = 0;
  for( std::string line; getline( input, line ); ) {
    

    bool parsingSuccessful = true;//reader.parse( line, root );     
    bytes += line.size();
if (!buf) {
      buf = strdup(line.c_str());
buf_size = line.size() + 1;
}
 else {

if (buf_size < line.size() + 1) {
  buf_size = line.size() + 1;
  buf = (char*)realloc(buf, buf_size);
}
memcpy(buf, line.c_str(), line.size() + 1);
}

if (document.ParseInsitu<0>(buf).HasParseError())
      {
        // report to the user the failure and their locations in the document.
    std::cout  << "Failed to parse configuration\n"
               <<std::endl;
    return 0;
      }

   }
  gettimeofday(&stop, NULL);
  time_t seconds = stop.tv_sec - start.tv_sec;
  if (seconds)
  std::cout << bytes/1024/1024/seconds << "MB/s " << bytes << "bytes in " << seconds << "seconds" << std::endl;

}
