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
    String s;
    while ((s = in.readLine()) != null && s.length() != 0) {
      parser = factory.createJsonParser(s);
      while(parser.nextToken() != null) {
         // do nothing
      }
    }
    long end = new Date().getTime();

    System.out.println("Processing took " + (end - start) + "ms");
  }
}
