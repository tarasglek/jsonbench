#include "json.h"

int main(int argc, char** argv) {
  
  FILE* input = stdin;
  if (argc == 2)
    input = fopen(argv[1], "r");

  size_t line_buf_size = 0;
  char *line_buf = NULL;
  JSONBench json;
  while(true) {
    ssize_t input_size = getline(&line_buf, &line_buf_size, input);
    if (input_size == -1)
      break;
    json.parse(line_buf, input_size);
  }
  free(line_buf);
}
