***Purpose
 To compare throughput of various json implementations and to come up with the fastest way to process json log files.

See Makefile for details.

This benchmarks reading of a line-separated json file. To run do something like
`
make node JSON=path/to/my/json/dump.txt
`
pipejson contains 2 executables that multiplex stdin across multiple forked processes. Unfortunately I could not make this work > 400MB/s consistently. 

The winning approach appears to be to shard data into multiple files and let Linux manage multiplexing IO.

lz4 appears to be the fastest compression algo. On my quadcore i5-3330S, I can decompress + process json at 450mb/s when using lz4 using the following command:
`
time make -j cxxp PIPE=lz4demo -d ~/work/data-telemetry/20130305-20130306.json.lz4 stdout
`

***Generating Sample Data

1. Enable telemetry in your browser.
2. Close browser
3. Go to your profile dir/saved-telemetry-pings copy those files somewhere safe
4. Write a program to cat those files into a few gb of input data
