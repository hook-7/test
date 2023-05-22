public class Test {

    public static void main(String[] args) {
        String frame = "72C09CDC51468AA80941705F0EE5D8E5A90E1DDA8EED18C504F736847FE826AAC847C0A8C02A9FF419F8A5533D861FDEB4A5AAA85F3EBB8DF71B4DBBFFC950A92E9059E124D9376C320E51B721F4C64188B849EACD6AA3176550AFD5988519718FB797890E45C8BDDEA42F1CCFB506EC89A05DC71D6B0000";
        frame = frame.replace(" ", "");
        // System.out.println(frame.substring(18, 20));
        byte[] data =  hexStringToByteArray(frame);
        String crc = crc16_xmodem(frame);
        System.err.println(crc);
        // System.out.println(Integer.toHexString(crc).toUpperCase());
        // int checksum = crc16_xmodem(data);
        // System.out.println( checksum);
        // String[] text = {"0000","1111"};
        // System.out.println(littleEndian(String.format("%08X", 100/ 2))); 
        // System.out.println(String.join("", text).length()/2);
        // System.out.println(crc32(frame));

    }

    public static String crc32(String frame) {

        byte[] data = hexStringToByteArray(frame);
        int crc = 0xFFFFFFFF;  // initialize CRC to all ones
        int polynomial = 0xEDB88320;  // CRC32 polynomial

        for (byte b : data) {
            crc ^= (b & 0xFF);

            for (int i = 0; i < 8; i++) {
                if ((crc & 1) != 0) {
                    crc = (crc >>> 1) ^ polynomial;
                } else {
                    crc >>>= 1;
                }
            }
        }
        crc = ~crc;  // invert all bits of the result
        return Integer.toHexString(crc).toUpperCase();
    }
    

    public static String crc16_xmodem(String frame) {
        System.err.println(frame);
        byte[] data = hexStringToByteArray(frame);
        int crc = 0x0000;
        int polynomial = 0x1021;

        for (byte b : data) {
            crc ^= (b & 0xFF) << 8;

            for (int i = 0; i < 8; i++) {
                if ((crc & 0x8000) != 0) {
                    crc = (crc << 1) ^ polynomial;
                } else {
                    crc <<= 1;
                }
            }
        }
        crc &= 0xFFFF;
        return String.format("%04X", crc & 0xFFFF);
    }

    public static byte[] hexStringToByteArray(String input) {
        int len = input.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            int high = Character.digit(input.charAt(i), 16);
            int low = Character.digit(input.charAt(i + 1), 16);
            data[i / 2] = (byte) ((high << 4) + low);
        }
        return data;
    }


    public static String littleEndian(String hexString) {
        StringBuilder _littleEndian = new StringBuilder();
        for (int i = hexString.length() - 2; i >= 0; i -= 2) {
            _littleEndian.append(hexString, i, i + 2);
        }
        return _littleEndian.toString();
    }

    public static String getFileLength(String[] data){
        int count = 0;
        for (String s:data
             ) {
            count += s.length() /2;
        }
        return littleEndian(String.format("%04X",count));
    }
    
}