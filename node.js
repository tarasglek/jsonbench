var fs = require('fs');
function readLines(input, onLine, onEnd) {
  var remaining = '';

  input.on('data', function(data) {
    remaining += data;
    var index = remaining.indexOf('\n');
    while (index > -1) {
      var line = remaining.substring(0, index);
      remaining = remaining.substring(index + 1);
      onLine(line);
      index = remaining.indexOf('\n');
    }
  });

  input.on('end', function() {
    if (remaining.length > 0) {
      onLine(remaining);
    }
    onEnd();
  });
}


var start = new Date();
var bytes = 0;

function processLine(data) {
  bytes += data.length
  JSON.parse(data)
}

var input = fs.createReadStream(process.argv[2]);


readLines(input, processLine, function() {
  var mseconds = (new Date() - start)
  console.log(1000*bytes/1024/1024/mseconds + "MB/s "+bytes+"bytes in "+mseconds/1000+"s" )
});
