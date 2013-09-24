local cjson = require "cjson"
start = os.time()
line = io.read("*line")
total = 0
while line ~= nil  do
  obj = cjson.decode(line)
  cjson.encode(obj)      
  total = total + string.len(line)
  line = io.read("*line")
end
time_taken = os.time() - start
if time_taken > 0 then
   print(total/1024/1024/ time_taken .. " MB/s\n")
end
