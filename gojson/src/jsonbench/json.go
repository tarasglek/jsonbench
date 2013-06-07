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
   dec := json.NewDecoder(os.Stdin)
   startTime := time.Now()
   for {
      var m map[string]interface{}
      if err := dec.Decode(&m); err == io.EOF {
         break
      } else if err != nil {
         log.Fatal(err)
      }
   }
   endTime := time.Since(startTime)
   fmt.Printf("Duration: %s", endTime)
}
