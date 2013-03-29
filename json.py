"""
Usage ./import.py telemetry_dump outdir/

Will produce outdir/histograms.txt, outdir/filter.json, outdir/<HISOTGRAM_NAME>.json

TODO: 

* switch from json to a binary encoding
* histograms.json should be replaced/enhanced with db schema reported by client
* include stddev, percentiles, etc where possible
"""
#!/usr/bin/python
import sys

try:
    import simplejson as json
    #import json
    from datetime import datetime
    start = datetime.now()
    java = False
except:
    #import com.xhaus.jyson.JysonCodec as json
    from com.xhaus.jyson import JysonCodec as json
    from java.util import Date
    start = Date()
    java = True
#import histogram_tools




INFILE = sys.argv[1]
if INFILE == "stdin":
    f = sys.stdin
else:
    f = open(INFILE)

bytes_read = 0

while True:
    oline = f.readline()
    l = len(oline)
    if l == 0:
        break
    bytes_read += len(oline)
    json.loads(oline)

if java:
    seconds = (Date().getTime() - start.getTime())/1000
else:
    seconds = (datetime.now() - start).total_seconds()
print str(bytes_read/1024/1024/seconds) + " MB/s %dbytes in %sseconds" % (bytes_read, seconds)
