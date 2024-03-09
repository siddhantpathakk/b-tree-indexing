package components.Database;

public class Block {
    private static int maximumRecordCount;
    private int currentRecordCount;
    private Record[] records;

    public Block(int BLOCK_SIZE) {
        this.currentRecordCount = 0;
        maximumRecordCount = BLOCK_SIZE / Record.getRecordSize();
        this.records = new Record[maximumRecordCount];
    }

    public boolean blockAvailable() {
        return currentRecordCount < maximumRecordCount;
    }

    public int insertRecord(Record rec) {
        for (int i = 0; i < records.length; i++) {
            if (records[i] == null) {
                records[i] = rec;
                this.currentRecordCount++;
                return i;
            }
        }
        return -1;
    }

    public void deleteRecord(int offset) {
        records[offset] = null;
        this.currentRecordCount--;
    }

    public int getCurrentRecordCount() {
        return this.currentRecordCount;
    }

    public static int getMaximumRecordCount() {
        return maximumRecordCount;
    }

    public Record getRecord(int offset) {
        return records[offset];
    }

    public Record getRecordFromBlock(int recordIndex) {
        return records[recordIndex];
    }

}