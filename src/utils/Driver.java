package utils;

import java.util.ArrayList;
import java.util.concurrent.TimeUnit;

import components.BPTree.*;
import components.Database.*;
import components.Database.Record;
import components.Nodes.*;

public class Driver {

    public static int indexCalls = 0;

    public static void runExperiments(Storage storage, int invalidDataCount, BPlusTree BPTree) {
        System.out.println("Total records read: " + storage.getNumberOfRecords());
        System.out.println(invalidDataCount + " records skipped due to invalid data");

        System.out.println("\nRunning experiments 1-5 (please wait for 1-2s before each experiment executes)");
        try {
            experiment1(storage);
            System.out.println("---------------------------------------------------");
            TimeUnit.SECONDS.sleep(1);

            experiment2(BPTree);
            System.out.println("---------------------------------------------------");
            TimeUnit.SECONDS.sleep(1);

            experiment3(storage, BPTree);
            System.out.println("---------------------------------------------------");
            TimeUnit.SECONDS.sleep(1);

            experiment4(storage, BPTree);
            TimeUnit.SECONDS.sleep(1);
            System.out.println("---------------------------------------------------");

            experiment5(storage, BPTree);

        } catch (InterruptedException e) {
            System.out.println("User interrupted program, exiting run time");
        }
        System.out.println("\n\n@@@@@ Execution complete @@@@@\n");
    }

    public static void experiment1(Storage storage) {
        System.out.println("\nEXPERIMENT 1: Store the data on the disk and report the stats");
        System.out.printf("Number of records: %d\n", storage.getNumberOfRecords());
        System.out.printf("Size of record: %d Bytes", Record.getRecordSize());
        System.out.printf("Number of records stored in a block: %d\n", Block.getMaximumRecordCount());
        System.out.printf("Number of blocks for storing data: %d\n", storage.getFilledBlocksCount());
    }

    public static void experiment2(BPlusTree BPTree) {
        System.out.println(
                "\nExperiment 2: build a B+ tree on the attribute \"numVote\" by inserting the records sequentially and report the statistics");
        System.out.println("Parameter n: " + BPlusTree.SizeofNode);
        System.out.printf("Number of Nodes in B+ tree: %d\n", BPTree.countNodes(BPlusTree.getRoot()));
        System.out.printf("Number of Levels in B+ tree: %d\n", BPTree.getDepth(BPlusTree.getRoot()));
        System.out.println("Content of the root node (only Keys): " + BPlusTree.getRoot().keys + "\n");
    }

    public static void experiment3(Storage storage, BPlusTree BPTree) {
        System.out.println(
                "\nExperiment 3: retrieve those movies with the \"numVotes\" equal to 500 and report the following statistics:");
        BPTFunctions helper = new BPTFunctions();
        long startTime = System.nanoTime();
        ArrayList<Address> addresses = BPTree.getAddresses((float) 500);
        long endTime = System.nanoTime();
        float totalNumVotes = 0;
        int recordCount = 0;
        ArrayList<Record> result = new ArrayList<>();
        if (addresses != null) {
            for (Address address : addresses) {
                Record record = storage.getRecord(address);
                result.add(record);
                totalNumVotes += record.getNumVotes();
                recordCount++;
            }
        }
        System.out.printf("Number of index nodes accessed by process: %d", helper.getNodeReads());
        System.out.printf("\nNumber of data blocks accessed by process: %d", storage.getBlockAccesses());
        System.out.printf("\n\n(Index Search) Number of records found: %d", recordCount);
        System.out.printf("\nAverage of NumVotes of returned records: %.2f",
                recordCount > 0 ? totalNumVotes / recordCount : 0);
        System.out.printf("\n\tRunning time: %.3f ms\n", (endTime - startTime) / 1_000_000.0);
        startTime = System.nanoTime();
        int blockAccesses = storage.bruteForceSearch(500, 500);
        endTime = System.nanoTime();
        System.out.printf("\nNumber of data blocks accessed by bruteforce: %d", blockAccesses);
        System.out.printf("\n\tRunning Time: %.3f ms\n\n", (endTime - startTime) / 1_000_000.0);
    }

    public static void experiment4(Storage storage, BPlusTree BPTree) {
        System.out.println("\n\nEXPERIMENT 4: Retrieve those records with 30,000 >= \"numVotes\" <= 40,000:");
        long startTime = System.nanoTime();
        ArrayList<Address> addresses = BPTFunctions.getAddressesForKeysBetween(BPlusTree.getRoot(), 30_000, 40_000);
        long endTime = System.nanoTime();
        double averageRating = 0;
        int recordCount = 0;
        ArrayList<Record> result = new ArrayList<>();
        if (addresses != null) {
            for (Address address : addresses) {
                Record record = storage.getRecord(address);
                result.add(record);
                averageRating += record.getAverageRating();
                recordCount++;
            }
        }
        System.out.printf("Number of index nodes accessed by process: %d", indexCalls);
        System.out.printf("\nNumber of data blocks accessed by process: %d", storage.getBlockAccesses());
        System.out.printf("\n\n(Index Search) Number of records found: %d", recordCount);
        System.out.printf("\nAverage of averageRatings of returned records: %.2f",
                recordCount > 0 ? averageRating / recordCount : 0);
        // running time = endTime - startTime (in nanoseconds)
        System.out.printf("\n\tRunning time: %.3f ms\n", (endTime - startTime) / 1_000_000.0);
        // point 5 brute-force searching
        startTime = System.nanoTime();
        int blockAccesses = storage.bruteForceSearch(30000, 40000);
        endTime = System.nanoTime();
        System.out.printf("\nNumber of data blocks accessed by bruteforce: %d", blockAccesses);
        System.out.printf("\n\tRunning Time: %.3f ms\n\n", (endTime - startTime) / 1_000_000.0);
    }

    public static void experiment5(Storage storage, BPlusTree BPTree) {
        System.out.println("\n\nEXPERIMENT 5: Delete those records with \"numVotes\" = 1,000:");
        long startTime = System.nanoTime();
        ArrayList<Float> keysToRemove = getKeysToRemove(BPlusTree.getRoot(), 1000, 1000);
        ArrayList<Address> addressesToRemove = new ArrayList<Address>();
        for (Float key : keysToRemove) {

            addressesToRemove.addAll(BPTree.removeKeys(BPlusTree.rootNode, null, -1, -1, key, key));
        }
        System.out.printf("Number of records to delete: %d\n", addressesToRemove.size());
        long endTime = System.nanoTime();
        System.out.printf("Number of Nodes in updated B+ tree: %d\n", BPTree.countNodes(BPlusTree.getRoot()));
        System.out.printf("Number of Levels in updated B+ tree: %d\n", BPTree.getDepth(BPlusTree.getRoot()));
        System.out.printf("\nContent of the root node of the updated B+ tree(only the keys): %s\n",
                BPlusTree.getRoot().keys);
        System.out.printf("\tRunning time: %.3f ms", (endTime - startTime) / 1_000_000.0);

        System.out.print("\n\nBrute-force range deletion:");
        startTime = System.nanoTime();
        int bruteForceAccessCount = storage.bruteForceSearch(1000, 1000);
        endTime = System.nanoTime();
        System.out.printf("\nNumber of data blocks that would be accessed by a brute-force: %d\n",
                bruteForceAccessCount);
        System.out.printf("\tRunning time: %.3f ms\n", (endTime - startTime) / 1_000_000.0);
    }

    public static ArrayList<Float> getKeysToRemove(NodeFunctions node, float lowerBound, float upperBound) {
        ArrayList<Float> keysToRemove = new ArrayList<Float>();
        while (!node.isLeaf()) {
            node = ((InternalNode) node).getChild(0);
        }
        LeafNode leafNode = (LeafNode) node;
        boolean flag = false;
        int pointer = 0;
        while (!flag && leafNode != null) {
            while (pointer < leafNode.keys.size()) {
                Float key = leafNode.keys.get(pointer);
                pointer += 1;
                if (key >= lowerBound && key <= upperBound) {
                    keysToRemove.add(key);
                } else if (key < lowerBound) {
                    pointer += 1;
                } else if (key > upperBound) {
                    flag = true;
                }
            }
            pointer = 0;
            leafNode = leafNode.getRightSibling();
        }
        System.out.printf("Number of keys to delete: %d\n", keysToRemove.size());
        return keysToRemove;
    }
}