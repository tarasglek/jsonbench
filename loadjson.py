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
import importlib
json = importlib.import_module(sys.argv[1])

try:
    from java.util import Date
    start = Date()
    jython = True
except ImportError:
    from datetime import datetime
    start = datetime.now()
    jython = False

f = sys.stdin

bytes_read = 0

while True:
    oline = f.readline()
    l = len(oline)
    if l == 0:
        break
    bytes_read += len(oline)
    json.loads(oline)

if jython:
    ms = (Date().getTime() - start.getTime())
else:
    delta = (datetime.now() - start)
    ms = delta.seconds * 1000 + delta.microseconds/1000

print str(1000*bytes_read/1024/1024/ms) + " MB/s %dbytes in %sseconds" % (bytes_read, ms/1000)
