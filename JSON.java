import java.io.*;
import com.fasterxml.jackson.core.JsonFactory;

public class JSON {
  public static void main(String[] args) throws IOException {
    BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
    JsonFactory factory = new JsonFactory();
    String s;
    while ((s = in.readLine()) != null && s.length() != 0) {
      factory.createJsonParser(s).readValueAsTree();
    }
  }
}
