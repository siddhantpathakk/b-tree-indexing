package components.index;

public class BPTHelper {

    private static int nodeReads;
    private static int nodeReadsEx4;
 
    public int getNodeReads() {
        return nodeReads;
    }


    static void addNodeReads() {
        nodeReads++;
    }

 
    public int getNodeReadsEx4() {
        return nodeReadsEx4;
    }


    static void addIndexNodeReads() {
        nodeReadsEx4++;
        addNodeReads();
    }

}
