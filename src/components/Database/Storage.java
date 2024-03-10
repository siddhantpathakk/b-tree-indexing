package components.Database;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;

public class Storage {

    private Block[] blocks;
    private Set<Integer> availableBlocks;
    private Set<Integer> filledBlocks;
    int diskSize;
    int blockSize;
    private int numRecords = 0;

    private static int blockAccesses = 0;

    public Storage(int diskSize, int blockSize) {
        this.diskSize = diskSize;
        this.blockSize = blockSize;
        this.blocks = new Block[diskSize / blockSize];
        this.availableBlocks = new HashSet<>();
        this.filledBlocks = new HashSet<>();
        for (int i = 0; i < blocks.length; i++) {
            blocks[i] = new Block(blockSize);
            availableBlocks.add(i);
        }
    }

    public Address writeRecordToStorage(Record record) {
        numRecords++;
        int blockPointer = getAvailableBlock();
        return this.insertRecordIntoBlock(blockPointer, record);
    }

    public int getNumberOfRecords() {
        return numRecords;
    }

    private int getAvailableBlock() {
        if (availableBlocks.isEmpty())
            return -1;
        return availableBlocks.iterator().next();
    }

    private Address insertRecordIntoBlock(int blockPointer, Record record) {
        if (blockPointer == -1)
            return null;
        int offset = blocks[blockPointer].insertRecord(record);
        filledBlocks.add(blockPointer);
        if (!blocks[blockPointer].blockAvailable())
            availableBlocks.remove(blockPointer);
        return new Address(blockPointer, offset);
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

    public Record getRecord(Address address) {
        Block block = getBlock(address.getBlockId());
        return block.getRecord(address.getOffset());
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

    public int bruteForceSearch(int numVotes, int maxNumberOfVotes) {
        Record record;
        float numVotesInRecord;
        int numBlockAccess = 0;
        ArrayList<Record> res = new ArrayList<>();
        for (Integer blockPointer : this.filledBlocks) {
            numBlockAccess++;
            Block block = this.blocks[blockPointer];
            int blockSize = block.getCurrentRecordCount();

            for (int offset = 0; offset < blockSize; offset++) {
                record = block.getRecord(offset);
                numVotesInRecord = record.getNumVotes();
                if (numVotesInRecord >= numVotes && numVotesInRecord <= maxNumberOfVotes) {
                    res.add(record);
                }
            }
        }
        if (res.isEmpty()) {
            System.out.println("\n(Bruteforce) No records found");
        } else {
            System.out.printf("\n(Bruteforce) No. of records found: %d", res.size());
        }
        return numBlockAccess;
    }
}