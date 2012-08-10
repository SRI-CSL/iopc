import java.util.prefs.Preferences;

public class TabPreferences {

    public static final double GOLDEN_MEAN = 1.61803399;
    public static final int    DEFAULT_H = 512;
    public static final int    DEFAULT_W = (int)(DEFAULT_H * GOLDEN_MEAN);
    
    //TabFrame
    public static final String W      = "Width of TabFrame";
    public static final String H      = "Height of TabFrame";
    public static final String X      = "X of TabFrame";
    public static final String Y      = "Y of TabFrame";
    
    //TabPanel
    public static final String DIV_V  = "V divider location";
    public static final String DIV_H  = "H divider location";
 
    public static final Preferences preferences = Preferences.userNodeForPackage(TabPreferences.class);




}
