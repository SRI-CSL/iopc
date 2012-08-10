import javax.swing.SwingUtilities;

public class PLA3 {

    private static void addDemoTabs(TabFrame tf){
        tf.addTabPanel(new TabPanel("Graph 1", "Main Dish"));
        tf.addTabPanel(new TabPanel("Graph 2", "Subnet of 1"));
        tf.addTabPanel(new TabPanel("Graph 3", "Findpath of 2"));
        tf.addTabPanel(new TabPanel("Graph 4", "Another Subnet of 1"));
        tf.addTabPanel(new TabPanel("Graph 5", "Compare of 1 & 3"));
    }

    public static void main (String [] args) {
        SwingUtilities.invokeLater(new Runnable(){
                public void run(){ 
                    TabFrame tf = new TabFrame();
                    addDemoTabs(tf);
                    tf.pack();
                    tf.setVisible(true);  
                }
            });
    }


}