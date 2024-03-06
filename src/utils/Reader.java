package utils;

import java.io.*;
import java.util.Scanner;

import components.BPTree.BPlusTree;
import components.DB.Address;
import components.DB.Database;
import components.DB.Record;

public class Reader {
    public static final int SizeofBlock = 200;
    public static final int PointerSize = 8;
    public static final int KeySize = 4;
    public static final int DiskCapacity = 200_000_000;

    public static void read() {
        String fileSeparator = System.getProperty("file.separator");
        String filePath = new File("").getAbsolutePath();
        filePath = filePath + fileSeparator + "data" + fileSeparator + "data.tsv";
        File file = new File(String.valueOf(filePath));

        try {

            if (file.exists()) {
                System.out.println("Reading data from " + String.valueOf(filePath));
                readFile(String.valueOf(filePath));
            } else {
                Scanner sc = new Scanner(System.in);
                System.out.print("Default file path failed! Please input the absolute file path of data.tsv: ");
                filePath = sc.nextLine();
                File dataFile = new File(String.valueOf(filePath));
                sc.close();

                if (dataFile.exists()) {
                    System.out.println("Reading data from " + String.valueOf(filePath));
                    readFile(String.valueOf(filePath));
                }
            }
        } catch (Exception e) {
            System.out.println(e);
        }
    }

    public static void readFile(String filePath) {
        try {
            String line;
            Database Database = new Database(DiskCapacity, SizeofBlock);
            BufferedReader reader = new BufferedReader(new FileReader(filePath));
            reader.readLine();
            int error = 0;

            BPlusTree BPTree = new BPlusTree();

            while ((line = reader.readLine()) != null) {
                String[] recordString = line.split("\t");
                try {
                    Record record = createRecord(recordString);
                    Address address = Database.writeRecordToStorage(record);
                    float key = record.getNumVotes();
                    BPTree.insertKeyAddrPair(key, address);
                } catch (Exception e) {
                    error++;
                    System.out.println("Invalid data found in recordString: " + line);
                    break;
                }

            }
            reader.close();
            Driver.runExperiments(Database, error, BPTree);

        } catch (Exception e) {
            System.out.println(e);
        }
    }

    public static Record createRecord(String[] recordString) {
        String tConst = recordString[0];
        float averageRating = Float.parseFloat(recordString[1]);
        int numVotes = Integer.parseInt(recordString[2]);
        return new Record(tConst, averageRating, numVotes);
    }

}