import java.util.zip.CRC32;

public class CRC32Example {
    public static void main(String[] args) {
        byte[] data = "Hello, world!".getBytes();

        CRC32 crc32 = new CRC32();
        crc32.update(data);
        long checksum = crc32.getValue();
     
        System.out.printf("CRC32 checksum: %s", String.format("%08X", checksum ));
    }
}
