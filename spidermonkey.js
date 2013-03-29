var start = new Date()
var bytes = 0;
var l;
while(l = readline()) {
  bytes += l.length
  JSON.parse(l)
}
var miliseconds = (new Date() - start)
print(1000*bytes/1024/1024/miliseconds + "MB/s "+bytes+"bytes in "+miliseconds/1000+"s" )
