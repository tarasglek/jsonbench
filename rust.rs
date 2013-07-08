extern mod extra;

fn main() {

    let i = std::io::stdin();
    let t1 = extra::time::get_time();
    let mut bytes_read = 0;
    while !i.eof() {
        let line = i.read_line();
        let json = extra::json::from_str(line);
        if line.len() != 0 {
            bytes_read += line.len();
            match (json) {
                Ok(value) => {}
                Err(e) => {
                    let mut tmp = ~"'";
                    tmp = tmp + line;
                    tmp = tmp + ~"'";
                    std::io::println(~"Error parsing " + tmp);
                }
            }
        }
    }
    let t2 = extra::time::get_time();

    let mut seconds = t2.sec - t1.sec;
    let mut nanos = t2.nsec;
    if nanos < t1.nsec {
        nanos += 1000000000;
        seconds -= 1;
    }

    nanos -= t1.nsec;
    let megabytes_read = bytes_read as float / 1024f / 1024f;
    let float_seconds = seconds as float + (nanos as float / 1000000000f);
    std::io::println(fmt!("Processing %.02f MB took %.02f seconds (%.02f MB/s).", megabytes_read, float_seconds, (megabytes_read / float_seconds)));
}
