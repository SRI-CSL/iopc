import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

import java.util.ArrayList;

import java.util.prefs.Preferences;

import g2d.jlambda.Closure;


public class TabFrame extends AppFrame { 

    private static final Preferences preferences = TabPreferences.preferences;

    //jlambda really shouldn't see these
    private final JTabbedPane tabs = new JTabbedPane();
    
    // jlambda closures to augment the construction of TabFrames ...
    static final ArrayList<Closure> creationClosures = new ArrayList<Closure>();

    public TabFrame(){
        super("TabFrame");
        tabs.setTabLayoutPolicy(JTabbedPane.WRAP_TAB_LAYOUT);
        int width = preferences.getInt(TabPreferences.W, TabPreferences.DEFAULT_W);
        int height = preferences.getInt(TabPreferences.H, TabPreferences.DEFAULT_H);
        int x = preferences.getInt(TabPreferences.X, 0);
        int y = preferences.getInt(TabPreferences.Y, 0);
        this.setLocation(x, y);
        this.setPreferredSize(new Dimension(width, height));
        this.add(tabs);

        this.addWindowListener(new WindowAdapter() {
                public void windowOpened(WindowEvent windowEvent) {
                    TabFrame.this.addComponentListener(new ComponentAdapter() {
                            public void componentMoved(ComponentEvent componentEvent) {
                                preferences.putInt(TabPreferences.X, TabFrame.this.getX());
                                preferences.putInt(TabPreferences.Y, TabFrame.this.getY());
                            }
                            public void componentResized(ComponentEvent componentEvent) {
                                preferences.putInt(TabPreferences.W, TabFrame.this.getWidth());
                                preferences.putInt(TabPreferences.H, TabFrame.this.getHeight());
                            }
                        });
                }
                public void windowClosing(WindowEvent windowEvent) {
                    preferences.putInt(TabPreferences.W, TabFrame.this.getWidth());
                    preferences.putInt(TabPreferences.H, TabFrame.this.getHeight());
                    preferences.putInt(TabPreferences.X, TabFrame.this.getX());
                    preferences.putInt(TabPreferences.Y, TabFrame.this.getY());
                }
            });
        
    }
    

    public TabFrame(TabPanel contents){
        this();
        addTabPanel(contents);
    }


    public static void addCreationClosure(Closure closure){
        creationClosures.add(closure);
    }
    
    private void onCreate(TabFrame newframe){
        for(Closure closure : creationClosures){
            closure.applyClosure(newframe);
        }
    }
    

    public int getTabPanelCount(){ return tabs.getTabCount();    }

    public int indexOfTabControl(TabControl control){
        return tabs.indexOfTabComponent(control);
    }

    public int indexOfTabPanel(TabPanel panel){
        return tabs.indexOfComponent(panel);
    }

    public TabPanel getTabPanelAt(int index){
        TabPanel retval = null;
        Component content = tabs.getComponentAt(index);
        if(content instanceof TabPanel){
            retval = (TabPanel)content;
        }
        return retval;
    }

    public void removeTabPanelAt(int index){
        tabs.remove(index);
        if(tabs.getTabCount() == 0){ dispose(); }
    }

    public void addTabPanel(TabPanel child){
        int index = tabs.getTabCount();
        tabs.addTab(child.title, null, child, child.toolTip);
        tabs.setTabComponentAt(index, new TabControl(this, child));
    }

    public void addTabPanel(int index, TabPanel child){
        tabs.insertTab(child.title, null, child, child.toolTip, index);
        tabs.setTabComponentAt(index, new TabControl(this, child));
    }


    public void insertBefore(int index, int position){
        if(position == index){ return; }
        int destination = position;
        TabPanel tab = getTabPanelAt(index);
        removeTabPanelAt(index);
        if(index < position){  destination--; }
        addTabPanel(destination, tab);
    }

    public void insertAfter(int index, int position){
        if(position == index){ return; }
        int destination = position;
        TabPanel tab = getTabPanelAt(index);
        removeTabPanelAt(index);
        if(position < index){  destination--; }
        addTabPanel(destination, tab);
    }

    public boolean containsLocationOnScreen(Point p){
        return contains(p.x - getX(), p.y - getY());
    }
    
    public String toString(){ return "TabFrame_" + id;  }

}

