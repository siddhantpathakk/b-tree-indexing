package storageComponent;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;


public class Database {
    
    private Block[] blocks;
    private Set<Integer> availableBlocks;
    private Set<Integer> filledBlocks;
    int diskSize;
    int blkSize;
    private int numRecords = 0;

    private static int blockAccesses = 0;

    public Database(int diskSize, int blkSize) {
        this.diskSize = diskSize;
        this.blkSize = blkSize;
        this.blocks = new Block[diskSize / blkSize];
        this.availableBlocks = new HashSet<>();
        this.filledBlocks = new HashSet<>();
        // all block are avail at the start
        for (int i = 0; i < blocks.length; i++) {
            blocks[i] = new Block(blkSize);
            availableBlocks.add(i);
        }
    }

    
    public Address writeRecordToStorage(Record rec) {
        numRecords++;
        int blockPtr = getFirstAvailableBlockId();
        return this.insertRecordIntoBlock(blockPtr, rec);
    }

    public int getNumberOfRecords() {
        return numRecords;
    }

    private int getFirstAvailableBlockId() {
        if (availableBlocks.isEmpty()) return -1;
        return availableBlocks.iterator().next();
    }

    private Address insertRecordIntoBlock(int blockPtr, Record rec) {
        if (blockPtr == -1) return null;
        int offset = blocks[blockPtr].insertRecord(rec);
        filledBlocks.add(blockPtr);
        if (!blocks[blockPtr].blockAvailable()) availableBlocks.remove(blockPtr);
        return new Address(blockPtr, offset);
    }

    public int getFilledBlocksCount() {
        return filledBlocks.size();
    }

    
    public int getBlockAccesses() {
        return blockAccesses;
    }


    private Block getBlock(int blockNumber) {
        Block block = blocks[blockNumber];
        blockAccesses++;
        return block;
    }

    public Record getRecord(Address add) {
        Block block = getBlock(add.getBlockId());
        return block.getRecord(add.getOffset());
    }


    public void deleteRecord(ArrayList<Address> addList) {
        for (Address add : addList) {
            int blockId = add.getBlockId();
            int offset = add.getOffset();
            Block block = getBlock(blockId);
            block.deleteRecord(offset);
            if (filledBlocks.contains(blockId)) {
                filledBlocks.remove(blockId);
            }
            availableBlocks.add(blockId);
        }
    }

    // bruteforce search, return number of block accesses
    public int bruteForceSearch(float fg_pct_home, float fg_pct_home_upper_bound) {
        Record rec;
        float recFgPct;
        int blkAccesses = 0;
        ArrayList<Record> res = new ArrayList<>();
        for (Integer blkPtr : this.filledBlocks) {
            blkAccesses++;
            Block block = this.blocks[blkPtr];
            int blockSize = block.getCurrSize();
            
            for (int offset = 0; offset < blockSize; offset++) {
                rec = block.getRecord(offset);
                recFgPct = rec.getFgPctHome();
                if (recFgPct >= fg_pct_home && recFgPct <= fg_pct_home_upper_bound) {
                    res.add(rec);
                }
            }
        }
        if (res.isEmpty()) {
            System.out.printf("\nNo records within range [%.2f, %.2f] found in db", fg_pct_home, fg_pct_home_upper_bound);
        } else{
            System.out.printf("\n(Bruteforce) No. of records found: %d", res.size());
        }
        return blkAccesses;
    }

    public void ex1() {
        System.out.println("\nEXPERIMENT 1: store the data from games.txt on the disk and report statistics:");
        System.out.printf("Number of records: %d\n", this.getNumberOfRecords());
        System.out.println(String.format("Size of record: %d Bytes", Record.getRecordSize()));
        System.out.printf("Number of records stored in a block: %d\n", Block.getmaxRecordCount());
        System.out.println(String.format("Number of blocks for storing data: %d\n", this.getFilledBlocksCount()));
    }
}