package storageComponent;


public class Address {
    int blockID;
    int offset;

    public Address(int blockID, int offset) {
        this.blockID = blockID;
        this.offset = offset;
    }
    
    public int getBlockId() {
        return blockID;
    }
    public int getOffset() {
        return offset;
    }

    @Override
    public String toString() {
        return String.format("BLOCK %d OFFSET %d", blockID, offset);
    }

}