extern mod std;
use core::io;
use core::str;
use std::json;
use std::time;

fn main() {

    let i = io::stdin();
    let t1 = time::get_time();
    let mut bytes_read = 0;
    while !i.eof() {
        let line = i.read_line();
        let json = json::from_str(line);
        if line.len() != 0 {
            bytes_read += str::len(line);
            match (json) {
                Ok(value) => {}
                Err(e) => {
                    let mut tmp = ~"'";
                    tmp+=(line);
                    tmp+=~"'";
                    io::println(~"Error parsing " + tmp);
                }
            }
        }
    }
    let t2 = time::get_time();

    let mut seconds = t2.sec - t1.sec;
    let mut nanos = t2.nsec;
    if nanos < t1.nsec {
        nanos += 1000000000;
        seconds -= 1;
    }

    nanos -= t1.nsec;
    let megabytes_read = bytes_read as float / 1024f / 1024f;
    let float_seconds = seconds as float + (nanos as float / 1000000000f);
    io::println(fmt!("Processing %.02f MB took %.02f seconds (%.02f MB/s).", megabytes_read, float_seconds, (megabytes_read / float_seconds)));
}
