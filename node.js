var fs = require('fs');
var start = new Date()
var bytes = 0;
function readLines(input, func) {
  var remaining = '';

  input.on('data', function(data) {
    remaining += data;
    var index = remaining.indexOf('\n');
    while (index > -1) {
      var line = remaining.substring(0, index);
      remaining = remaining.substring(index + 1);
      func(line);
      index = remaining.indexOf('\n');
    }
  });

  input.on('end', function() {
    if (remaining.length > 0) {
      func(remaining);
    }
             var mseconds = (new Date() - start)
             console.log(1000*bytes/1024/1024/mseconds + "MB/s "+bytes+"bytes in "+mseconds/1000+"s" )

  });
}


function func(data) {
  bytes += data.length
  JSON.parse(data)
}

var input = fs.createReadStream(process.argv[2]);
readLines(input, func);

