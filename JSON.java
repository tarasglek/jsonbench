import java.io.*;
import java.util.Date;
import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonParseException;
import com.fasterxml.jackson.core.JsonParser;

public class JSON {
  public static void main(String[] args) throws IOException, JsonParseException {
    long start = new Date().getTime();
    BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
    JsonFactory factory = new JsonFactory();
    JsonParser parser = null;
    long bytesRead = 0;
    String s;
    while ((s = in.readLine()) != null && s.length() != 0) {
      bytesRead += s.length();
      parser = factory.createJsonParser(s);
      while(parser.nextToken() != null) {
         // do nothing
      }
    }
    long end = new Date().getTime();

    long duration = end - start;
    double mbPerSec = ((double)bytesRead / 1024.0 / 1024.0) / (duration / 1000.0);
    System.out.println(String.format("Processing %d bytes took %d ms (%.2f MB/s)", bytesRead, duration, mbPerSec));
  }
}
