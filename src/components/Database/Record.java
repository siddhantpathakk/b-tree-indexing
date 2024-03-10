package components.Database;

public class Record {
    private String tConst; 
    private float averageRating;
    private float numVotes; 

    private static int RECORD_SIZE = 18;

    public Record(String tConst, float averageRating, int numVotes) {
        this.tConst = tConst;
        this.averageRating = averageRating;
        this.numVotes = numVotes;
    }

    public Record() {
        this.tConst = "";
        this.averageRating = 0;
        this.numVotes = 0;
    }

    public static int getRecordSize() {
        return RECORD_SIZE;
    }

    // Getters
    public String getTConst() {
        return tConst;
    }

    public float getAverageRating() {
        return averageRating;
    }

    public float getNumVotes() {
        return numVotes;
    }

    // Setter
    public void setTConst(String tConst) {
        this.tConst = tConst;
    }

    public void setAverageRating(float averageRating) {
        this.averageRating = averageRating;
    }

    public void setNumVotes(int numVotes) {
        this.numVotes = numVotes;
    }

    @Override
    public String toString() {
        return "Record{" +
                "tConst='" + tConst + '\'' +
                ", averageRating=" + averageRating +
                ", numVotes=" + numVotes +
                '}';
    }
}
