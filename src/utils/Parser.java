package utils;

import components.index.BPlusTree;

import java.util.Date;
import java.util.Scanner;
import java.util.concurrent.TimeUnit;
import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;

import components.storage.Address;
import components.storage.Record;
import components.storage.Database;;;

public class Parser {
    public static final int BLOCK_SIZE = 400;
    public static final int POINTER_SIZE = 8; // pointer size for 64 bit systems = 8 bytes
    public static final int KEY_SIZE = 4; // type(key) = float 4 bytes
    private static int counter = 0;

    public static void readTXTFile(String filePath, int diskCapacity) {
        try {
            String line;
            Database db = new Database(diskCapacity, BLOCK_SIZE);
            BufferedReader reader = new BufferedReader(new FileReader(filePath));
            reader.readLine();
            int invalidDataCount = 0;

            BPlusTree bPlusTree = new BPlusTree();

            while ((line = reader.readLine()) != null) {
                // counter++;
                // if (counter % 10000 == 0)
                //     System.out.println(counter + " data rows read");
                String[] tuple = line.split("\t");
                try {
                    Record row = parseTuple(tuple);                
                    Address addr = db.writeRecordToStorage(row);
                    float key = row.getNumVotes();
                    bPlusTree.insertKeyAddrPair(key, addr); // do multiple inserts, another option is bulk loading
                } catch (Exception e) { // handles empty cells + parse exception
                    invalidDataCount++;
                    System.out.println("Invalid data found in tuple: " + line);
                    break;
                }
                
            }
            reader.close();
            System.out.println("Total records read: " + db.getNumberOfRecords());
            System.out.println(invalidDataCount + " tuples skipped due to invalid data");
            
            System.out.println("\nRunning experiments 1-4 (please wait for 1-2s before each experiment executes)");
            try {
                db.ex1();
                System.out.println("===================================");
                TimeUnit.SECONDS.sleep(2);

                BPlusTree.ex2(bPlusTree);
                System.out.println("===================================");
                TimeUnit.SECONDS.sleep(2);

                BPlusTree.ex3(db, bPlusTree);
                System.out.println("===================================");
                TimeUnit.SECONDS.sleep(2);

                BPlusTree.ex4(db, bPlusTree);
                TimeUnit.SECONDS.sleep(2);
                System.out.println("===================================");

                bPlusTree.ex5(db, bPlusTree);
                System.out.println("===================================");

            } catch (InterruptedException e) {
                System.out.println("User interrupted program, exiting run time");
            }
            System.out.println("\n\n@@@@@ Execution complete @@@@@\n");
            
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static Record parseTuple(String[] tuple) throws ParseException {
        String tConst = tuple[0];
        float averageRating = Float.parseFloat(tuple[1]);
        int numVotes = Integer.parseInt(tuple[2]);
        return createRecord(tConst, averageRating, numVotes); 
    }

    public static Record createRecord(String tConst, float averageRating, int numVotes) {
        return new Record(tConst, averageRating, numVotes);
    }

}