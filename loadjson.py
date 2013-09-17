#!/usr/bin/python
import sys
import importlib
# : because . is used for module names in python
[JSON_MODULE, JSON_DECODE_FUNC] = sys.argv[1].split(":")
json = importlib.import_module(JSON_MODULE)

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

loads = getattr(json, JSON_DECODE_FUNC)

dumps = None

if len(sys.argv) > 2:
    dumps = getattr(json, sys.argv[2])

while True:
    oline = f.readline()
    l = len(oline)
    if l == 0:
        break
    bytes_read += l
    obj = loads(oline)
    if dumps != None:
        dumps(obj)

if jython:
    ms = (Date().getTime() - start.getTime())
else:
    delta = (datetime.now() - start)
    ms = delta.seconds * 1000 + delta.microseconds/1000

print str(1000*bytes_read/1024/1024/ms) + " MB/s %dbytes in %sseconds" % (bytes_read, ms/1000)
