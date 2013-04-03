all: shmjson pipejson

shmjson: shmjson.cpp unix_util.h
	g++ -O3 -Wall -g -o $@ $< -lrt -fno-omit-frame-pointer

pipejson: pipejson.cpp
	g++ -O3 -Wall -g -o $@ $< -lrt -fno-omit-frame-pointer

clean:
	rm -f pipejson shmjson
