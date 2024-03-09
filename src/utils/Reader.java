package utils;

import java.io.*;
import java.util.Scanner;

import components.BPTree.BPlusTree;
import components.Database.Address;
import components.Database.Record;
import components.Database.Storage;

public class Reader {
    public static final int SizeofBlock = 200;
    public static final int SizeofPointer = 8;
    public static final int SizeofKey = 4;
    public static final int DiskCapacity = 200000000;

    public static void read() {
        String fileSeparator = System.getProperty("file.separator");
        String filePath = new File("").getAbsolutePath();
        filePath = filePath + fileSeparator + "data" + fileSeparator + "data.tsv";
        File file = new File(String.valueOf(filePath));

        try {

            if (file.exists()) {
                System.out.println("Fetching data from " + String.valueOf(filePath));
                readFile(String.valueOf(filePath));
            } else {
                Scanner sc = new Scanner(System.in);
                System.out.print("Cannot find file path, please enter manually: ");
                filePath = sc.nextLine();
                File dataFile = new File(String.valueOf(filePath));
                sc.close();

                if (dataFile.exists()) {
                    System.out.println("Fetching data from " + String.valueOf(filePath));
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
            Storage storage = new Storage(DiskCapacity, SizeofBlock);
            BufferedReader reader = new BufferedReader(new FileReader(filePath));
            reader.readLine();
            int error = 0;

            
            BPlusTree BPTree = new BPlusTree();

            while ((line = reader.readLine()) != null) {
                String[] recordString = line.split("\t");
                try {
                    Record record = createRecord(recordString);
                    Address address = storage.writeRecordToStorage(record);
                    float key = record.getNumVotes();
                    BPTree.insertKeyAddrPair(key, address);
                } catch (Exception e) {
                    error++;
                    System.out.println("Invalid data found in line: " + line);
                    break;
                }

            }
            reader.close();
            Driver.runExperiments(storage, error, BPTree);

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