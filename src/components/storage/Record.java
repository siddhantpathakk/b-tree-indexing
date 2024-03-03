package storageComponent;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

// Total size of record: 20 bytes
public class Record {
    private int gameDateEst; // 4 bytes
    private int teamIdHome; // 4 bytes
    private byte ptsHome; // 1 byte
    private float fgPctHome; // 4 bytes
    private float fg3PctHome; // 4 bytes
    private byte astHome; // 1 byte 
    private byte rebHome; // 1 byte
    private byte homeTeamWins; // 1 byte
    private static int RECORD_SIZE = 20;

    public Record(int gameDateEst, int teamIdHome, byte ptsHome, 
                  float fgPctHome, float fg3PctHome, 
                  byte astHome, byte rebHome, byte homeTeamWins) {
        this.gameDateEst = gameDateEst; 
        this.teamIdHome = teamIdHome;
        this.ptsHome = ptsHome;
        this.fgPctHome = fgPctHome;
        this.fg3PctHome = fg3PctHome;
        this.astHome = astHome;
        this.rebHome = rebHome;
        this.homeTeamWins = homeTeamWins;
    }
    

    // This is assuming all records must be NOT NULL
    public Record() {
        this.gameDateEst = 0;
        this.teamIdHome = 0;
        this.ptsHome = 0;
        this.fgPctHome = 0;
        this.fg3PctHome = 0;
        this.astHome = 0;
        this.rebHome = 0;
        this.homeTeamWins = 0;
    }

    public static int getRecordSize() {
        return RECORD_SIZE;
    }

    // Getters
    public int getGameDateEst() {
        return gameDateEst;
    }

    public int getTeamIdHome() {
        return teamIdHome;
    }

    public byte getPtsHome() {
        return ptsHome;
    }

    public float getFgPctHome() {
        return fgPctHome;
    }

    public float getFg3PctHome() {
        return fg3PctHome;
    }

    public byte getAstHome() {
        return astHome;
    }

    public byte getRebHome() {
        return rebHome;
    }

    public byte getHomeTeamWins() {
        return homeTeamWins;
    }

    // Setter 
    public void setGameDateEst(int gameDateEst) {
        this.gameDateEst = gameDateEst;
    }

    public void setTeamIdHome(int teamIdHome) {
        this.teamIdHome = teamIdHome;
    }

    public void setPtsHome(byte ptsHome) {
        this.ptsHome = ptsHome;
    }

    public void setFgPctHome(int fgPctHome) {
        this.fgPctHome = fgPctHome;
    }

    public void setFg3PctHome(int fg3PctHome) {
        this.fg3PctHome = fg3PctHome;
    }

    public void setAstHome(byte astHome) {
        this.astHome = astHome;
    }

    public void setRebHome(byte rebHome) {
        this.rebHome = rebHome;
    }

    public void setHomeTeamWins(byte homeTeamWins) {
        this.homeTeamWins = homeTeamWins;
    }

    @Override
    public String toString() {
        return "Record {" +
               "\n\tGAME_DATE_EST: " + intToDate(gameDateEst) + 
               "\n\tTEAM_ID_home: " + teamIdHome +
               "\n\tPTS_home: " + ptsHome +
               "\n\tFG_PCT_home: " + fgPctHome +
               "\n\tFG3_PCT_home: " + fg3PctHome +
               "\n\tAST_home: " + astHome +
               "\n\tREB_home: " + rebHome +
               "\n\tHOME_TEAM_WINS: " + homeTeamWins +
               "\n}";
    }
    
    public static String intToDate(int epochTime) {
        Date date = new Date((long) epochTime * 1000);
        DateFormat formatter = new SimpleDateFormat("dd/MM/yyyy");
        return formatter.format(date);
    }
    
}

