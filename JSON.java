import java.io.*;
import java.util.Date;
import com.fasterxml.jackson.core.*;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ObjectNode;

public class JSON {
  public static void main(String[] args) throws IOException, JsonParseException {
    boolean tokenizeOnly = false;
    if (args.length >= 1 && "tokenize".equalsIgnoreCase(args[0]))
       tokenizeOnly = true;

    System.out.println("Running " + (tokenizeOnly ? "tokenize" : "full") + " test");

    BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
    long bytesRead = 0;
    String s;

    long start = new Date().getTime();
    if (tokenizeOnly) {
      JsonFactory factory = new JsonFactory();
      JsonParser parser = null;
      while ((s = in.readLine()) != null && s.length() != 0) {
        bytesRead += s.length();
        parser = factory.createJsonParser(s);
        while(parser.nextToken() != null) {
           // do nothing
        }
      }
    } else {
      ObjectMapper jsonMapper = new ObjectMapper();
      ObjectNode document;
      while ((s = in.readLine()) != null && s.length() != 0) {
        bytesRead += s.length();
        document = jsonMapper.readValue(s, ObjectNode.class);
      }
    }
    long end = new Date().getTime();

    long duration = end - start;
    double durationSec = (duration / 1000.0);
    double mbPerSec = ((double)bytesRead / 1024.0 / 1024.0) / durationSec;
    System.out.println(String.format("%.2f MB/s %d bytes in %.2f s", mbPerSec, bytesRead, durationSec));
  }
}
