
import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import java.util.prefs.*;
import javax.swing.*;


public class TabPanel extends JPanel {
    public final String title;
    public final String toolTip;

    final JMenuBar menubar = new JMenuBar();
    final ContentPanel content;

    TabPanel(String title, String toolTip){
        content = new ContentPanel(this, title);
        JMenu menu = new JMenu(title);
        JMenuItem item = new JMenuItem("Menu Item");
        this.title = title;
        this.toolTip = toolTip;
        menu.add(item);
        menubar.add(menu);
        setLayout(new BorderLayout());
        add(menubar, BorderLayout.NORTH);
        add(content, BorderLayout.CENTER);
    }

    
}



class ContentPanel  extends JPanel {
    private static final Preferences preferences = TabPreferences.preferences;
    final TabPanel panel;
    final JToolBar toolBar = new JToolBar();
    final JSplitPane splitPaneV = new JSplitPane(JSplitPane.VERTICAL_SPLIT, false, new JPanel(), new JPanel());
    final JSplitPane splitPaneH = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, false, new JPanel(), splitPaneV);

    
    ContentPanel(final TabPanel panel, final String title){
        setLayout(new BorderLayout());
        this.panel = panel;
        toolBar.add(new AbstractAction("Press: " + title){
                public void actionPerformed(ActionEvent event){
                    TabFrame frame = TabUtils.getTabFrame(toolBar);
                    if(frame != null){
                        TabFrame tf = (TabFrame)frame;
                        int index = tf.indexOfTabPanel(panel);
                        if(index != -1){
                            String childTitle = "C(" + panel.title + ")";
                            String childTip = "Child of " + panel.toolTip.toLowerCase();
                            tf.addTabPanel(index + 1, new TabPanel(childTitle, childTip));
                        }
                    }

                }
                
            });

        
        installDivider(splitPaneH, TabPreferences.DIV_H);
        installDivider(splitPaneV, TabPreferences.DIV_V);

 

        add(toolBar, BorderLayout.NORTH);
        add(splitPaneH, BorderLayout.CENTER);
    }


    private void installDivider(final JSplitPane splitPane, final String prefKey){
        int ploc = preferences.getInt(prefKey, 0);
        splitPane.setDividerLocation(ploc);
        splitPane.addPropertyChangeListener(JSplitPane.DIVIDER_LOCATION_PROPERTY,
                                            new PropertyChangeListener() {
                                                public void propertyChange(PropertyChangeEvent e) {
                                                    int loc = splitPane.getDividerLocation();
                                                    preferences.putInt(prefKey, splitPane.getDividerLocation());
                                                }});
    }

}

