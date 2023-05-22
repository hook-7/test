import java.util.Arrays;
import java.util.List;


public class Req {
   public static void main(String[] args) {
    String[] split = "1,2,3,4,5".split(",");
    List<String> list = Arrays.asList(split);
    list.stream().filter(a -> Integer.parseInt(a) > 2).forEach(System.out::println);;

   }
}
