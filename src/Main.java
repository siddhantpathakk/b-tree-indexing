import utils.Parser;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;


public class Main {
    private static final int DEFAULT_MAX_DISK_CAPACITY = 100 * (int) (Math.pow(10, 6));

    public static void main(String[] args) throws Exception {

        String separator = System.getProperty("file.separator");
        String filePath = new File("").getAbsolutePath();
        filePath = filePath.concat(separator + "DBSP" + separator + "project1" + separator + "games.txt");

        File file = new File(String.valueOf(filePath));
        if (file.exists()) {
            System.out.println("Reading data from " + String.valueOf(filePath));
            int diskSize = DEFAULT_MAX_DISK_CAPACITY;
            Parser.readTXTFile(String.valueOf(filePath), diskSize);
        } else if (!file.exists()) {
            Scanner sc = new Scanner(System.in);
            System.out.print("Default file path failed! Please input the absolute file path of games.txt: ");
            filePath = sc.nextLine();
            File newFileCheck = new File(String.valueOf(filePath));
            if (newFileCheck.exists()) {
                System.out.println("Reading data from " + String.valueOf(filePath));
                int diskSize = DEFAULT_MAX_DISK_CAPACITY;
                Parser.readTXTFile(String.valueOf(filePath), diskSize);
            }
        } else {
            throw new FileNotFoundException("File does not exist!");
        }
    }
}
