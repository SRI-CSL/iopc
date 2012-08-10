import java.awt.*;
import java.awt.event.*;

import java.util.*;

import javax.swing.*;

public class TabUtils {



    private static final HashMap<Integer, AppFrame> idFrameMap = new HashMap<Integer, AppFrame>();

    public static TabFrame getTabFrame(Container cont){
        TabFrame frame = null;
        Container p = SwingUtilities.getAncestorOfClass(TabFrame.class,  cont);
        if(p != null){ frame = (TabFrame) p; }
        return frame;
    }


    public static void register(AppFrame tframe){
        Integer key = new Integer(tframe.id);
        if(idFrameMap.containsKey(key)){
            throw new IllegalArgumentException("Can't re-register a AppFrame!");
        }
        idFrameMap.put(key, tframe);
        houseKeeping();
    }

    public static void unregister(AppFrame tframe){
        Integer key = new Integer(tframe.id);
        if(!idFrameMap.containsKey(key)){
            throw new IllegalArgumentException("Can't unregister a unregistered AppFrame!");
        }
        idFrameMap.remove(key);
        houseKeeping();
    }


    public static TabFrame getTabFrameAtLocation(Point locationOnScreen){
        TabFrame retval = null;
        Iterator<Map.Entry<Integer, AppFrame>> it = idFrameMap.entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry<Integer, AppFrame> pairs = it.next();
            AppFrame frame = pairs.getValue();
            if(frame instanceof TabFrame){
                TabFrame candidate = (TabFrame)frame;
                if(candidate.containsLocationOnScreen(locationOnScreen)){
                    retval = candidate;
                }
            }
        }
        return retval;
    }
    

    //soonish the TabPanels should become IOPGraphs i.e.:
    //  public static IOPGraph launchTab(IOPGraph parent, String title, String toolTip)
    //
    public static TabPanel launchTab(TabPanel parent, String title, String toolTip){
        TabPanel retval =  new TabPanel(title, toolTip);
        TabFrame tf = null;
        if(parent == null){
            tf = new TabFrame(retval);
            tf.pack();
            tf.setVisible(true);
        } else {
            tf = getTabFrame(parent);
            int index = tf.indexOfTabPanel(parent);
            tf.addTabPanel(index + 1, retval);
        }
        return retval;
    }
    

    public static void launchTab(TabFrame oldFrame, int index, Point locationOnScreen){
        if(oldFrame == null){ return; }
        int count = oldFrame.getTabPanelCount();
        //don't waste energy on silly requests
        if((count <= 1) || (index == -1)){ return; }
        TabPanel content = oldFrame.getTabPanelAt(index);
        oldFrame.removeTabPanelAt(index);
        TabFrame child = new TabFrame(content);
        child.pack();
        child.setLocation(locationOnScreen);
        child.setVisible(true);
    }

    public static void transferTab(TabFrame oldFrame, int index, TabFrame newFrame){
        if(oldFrame == null){ return; }
        if(newFrame == null){  return; }
        //tab docking 
        int tabCount = oldFrame.getTabPanelCount(); 
        if((index < 0) || (index >= tabCount)){
            throw new IllegalArgumentException("Bad tab index: " + index + ", tabCount = " + tabCount);
        }
        TabPanel content = oldFrame.getTabPanelAt(index);
        oldFrame.removeTabPanelAt(index);
        newFrame.addTabPanel(content);
    }

    private static void houseKeeping(){
        constructWindowsMenu();
    }
    
    private static void constructWindowsMenu(){
        Iterator<Map.Entry<Integer, AppFrame>> it = idFrameMap.entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry<Integer, AppFrame> pairs = it.next();
            AppFrame candidate = pairs.getValue();
            constructWindowsMenu(candidate);
        }
    }


    
    private static void constructWindowsMenu(AppFrame frame){
        final JMenu wmenu = frame.windowsMenu;
        wmenu.removeAll();
        Iterator<Map.Entry<Integer, AppFrame>> it = idFrameMap.entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry<Integer, AppFrame> pairs = it.next();
            AppFrame candidate = pairs.getValue();
            addWindowMenuItem(wmenu, frame, candidate);
        }
    }

    private static void addWindowMenuItem(JMenu wmenu, final AppFrame frame, final AppFrame candidate){
        String title = candidate.getTitle();
        if(frame.id == candidate.id){ title += " *"; }
        AbstractAction action = new AbstractAction(title){
                public void actionPerformed(ActionEvent event){  candidate.toFront();   }
            };
        wmenu.add(new JMenuItem(action));
    }



    
}
    
