package utils;

import java.util.concurrent.TimeUnit;
import java.io.*;
import java.text.ParseException;

import components.BPTree.BPlusTree;
import components.DB.Address;
import components.DB.Database;
import components.DB.Record;;;

public class Reader {
    public static final int SizeofBlock = 200;
    public static final int PointerSize = 8; // pointer size for 64 bit systems = 8 bytes
    public static final int KeySize = 4; // type(key) = float 4 bytes
    public static final int DiskCapacity = 200_000_000;
  
    public static void read(String filePath, int diskCapacity) {
        try {
            String line;
            Database db = new Database(diskCapacity, SizeofBlock);
            BufferedReader reader = new BufferedReader(new FileReader(filePath));
            reader.readLine();
            int invalidDataCount = 0;

            BPlusTree bPlusTree = new BPlusTree();

            while ((line = reader.readLine()) != null) {
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
            
            System.out.println("\nRunning experiments 1-5 (please wait for 1-2s before each experiment executes)");
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