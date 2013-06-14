package main

import (
   "encoding/json"
   "fmt"
   "io"
   "os"
   "log"
   "time"
)

func main() {
   if len(os.Args) < 2 {
      log.Fatal("Usage: %s inputfile\n", os.Args[0])
   }
   file, err := os.Open(os.Args[1])
   if err != nil {
      log.Printf("Usage: %s inputfile\n", os.Args[0])
      log.Fatal(err)
   }
   dec := json.NewDecoder(file)
   startTime := time.Now()
   for {
      // Parse the JSON but throw away all the data
      var m struct{}
      if err := dec.Decode(&m); err == io.EOF {
         break
      } else if err != nil {
         log.Fatal(err)
      }
   }
   duration := time.Since(startTime)
   fileInfo, err := file.Stat()
   if err != nil {
      log.Fatal(err)
   }
   speed := float32(fileInfo.Size()) / 1024.0 / 1024.0 / float32(duration.Seconds())
   fmt.Printf("Duration: %s, %.02f MB/s", duration, speed)
}
