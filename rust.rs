extern mod std;
use core::io;
use std::json;

fn main() {

    let i = io::stdin();
    while !i.eof() {
        let line = i.read_line();
        let json = json::from_str(line);
        if line.len() != 0 {
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
}
