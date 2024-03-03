package storageComponent;


public class Block {
    private static int maxRecordCount;
    private int currRecordCount;
    private Record[] records;

    public Block(int BLOCK_SIZE) {
        this.currRecordCount = 0;
        this.maxRecordCount = BLOCK_SIZE / Record.getRecordSize();
        this.records = new Record[this.maxRecordCount];
    }

    public boolean blockAvailable() {
        return currRecordCount < maxRecordCount;
    }

    public int insertRecord(Record rec) {
        for (int i = 0; i < records.length; i++) {
            if (records[i] == null) {
                records[i] = rec;
                this.currRecordCount++;
                return i;
            }
        }
        // no space
        return -1;
    }

    public void deleteRecord(int offset) {
        records[offset] = null;
        this.currRecordCount--;
    }
    

    // GETTERS AND (NO) SETTERS
    public int getCurrSize() {
        return this.currRecordCount;
    }

    public static int getmaxRecordCount() {
        return maxRecordCount;
    }

    public Record getRecord(int offset) {
        return records[offset];
    }
    
    public Record getRecordFromBlock(int recordIdx) {
        return records[recordIdx];
    }

}