import javax.swing.*;
import javax.swing.*;
import javax.swing.plaf.basic.BasicButtonUI;
import java.awt.*;
import java.awt.event.*;

/**
 * Component to be used as tabComponent;
 * Contains a JLabel for the text and two buttons for control
 */ 
public class TabControl extends  JPanel {
    //owner 
    private final TabFrame owner;
    private final TabPanel panel;

    //cursor magic 
    protected static final Cursor hand = new Cursor(Cursor.HAND_CURSOR);
    protected static final Cursor cursor = new Cursor(Cursor.DEFAULT_CURSOR);
    

    public TabControl(final TabFrame owner, final TabPanel panel) {
        //unset default FlowLayout' gaps
        super(new FlowLayout(FlowLayout.LEFT, 0, 0));
        if (owner == null) {
            throw new NullPointerException("TabFrame is null");
        }

        this.owner = owner;
        this.panel = panel;

        setOpaque(false);

        //make JLabel read titles from TabPanel
        JLabel label = new JLabel() {
                public String getText() {
                    int i = getIndex();
                    if (i != -1) { return panel.title; } 
                    return null;
                }

                public String toString(){ return "TabLabel_" + getIndex(); }
                
        };

        add(new TabGripper());

        add(label);
        //add more space between the label and the buttons
        label.setBorder(BorderFactory.createEmptyBorder(0, 5, 0, 5));
        //tab button
        JButton button = new TabCloseButton();
        add(button);
        //add more space to the top of the component
        setBorder(BorderFactory.createEmptyBorder(2, 0, 0, 0));
    }

    public int getIndex(){ return owner.indexOfTabControl(this);  }

    private class TabGripper extends JButton  implements  TabInsertionHandler   { 

        TabDragListener tabDragListener = new TabDragListener();

        public TabGripper(){
            int size = 17;
            setPreferredSize(new Dimension(size, size));
            setToolTipText("launch this tab in a new window");
            setUI(new BasicButtonUI());
            //Make it transparent
            setContentAreaFilled(false);
            setFocusable(false);
            setBorder(BorderFactory.createEtchedBorder());
            setBorderPainted(false);
            //rollover highlighter
            addMouseListener(buttonMouseListener);
            //drag and dropper  ....
            addMouseListener(tabDragListener);
            addMouseMotionListener((MouseMotionListener)tabDragListener);

            setRolloverEnabled(true);
        }

        public String toString(){ return "TabGripper_" + getIndex(); }

        public void handleInsertion(int index){ owner.insertBefore(index, getIndex()); }
        
    }


    private class TabCloseButton extends JButton implements ActionListener, TabInsertionHandler {
        public TabCloseButton() {
            int size = 17;
            setPreferredSize(new Dimension(size, size));
            setToolTipText("close this tab");
            //Make the button looks the same for all Laf's
            setUI(new BasicButtonUI());
            //Make it transparent
            setContentAreaFilled(false);
            //No need to be focusable
            setFocusable(false);
            setBorder(BorderFactory.createEtchedBorder());
            setBorderPainted(false);
            //rollover highlighter
            addMouseListener(buttonMouseListener);
            setRolloverEnabled(true);
            //Close the proper tab by clicking the button
            addActionListener(this);
        }

        public void actionPerformed(ActionEvent e) {
            int i = owner.indexOfTabControl(TabControl.this);
            owner.removeTabPanelAt(i);
        }

        //we don't want to update UI for this button
        public void updateUI() {  }

        //paint the cross
        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            Graphics2D g2 = (Graphics2D) g.create();
            //shift the image for pressed buttons
            if (getModel().isPressed()) {
                g2.translate(1, 1);
            }
            g2.setStroke(new BasicStroke(1.5f, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));
            g2.setColor(Color.BLACK);
            if (getModel().isRollover()) {
                g2.setColor(Color.RED);
            }
            int offset = 5;
            g2.drawLine(offset, offset, getWidth() - offset - 1, getHeight() - offset - 1);
            g2.drawLine(getWidth() - offset - 1, offset, offset, getHeight() - offset - 1);
            g2.dispose();
        }

        
        public String toString(){ return "TabCloseButton_" + getIndex(); }


        public void handleInsertion(int index){ owner.insertAfter(index, getIndex()); }
        

    }




    private class TabDragListener extends MouseAdapter implements MouseListener, MouseMotionListener {
        DragWindow dragWindow;
        boolean visible = false;
        
        
        private DragWindow createDragWindow(){
            Window frame = TabUtils.getTabFrame(TabControl.this);
            DragWindow dragWindow = new DragWindow(frame, TabControl.this.getLocationOnScreen());
            return dragWindow;
        }
        
        public void mouseDragged(MouseEvent e){ 
            if(dragWindow == null){
                dragWindow = createDragWindow();
            }
            if(!visible){
                visible = true;
                dragWindow.moveTo(e.getX(), e.getY());
                dragWindow.setVisible(true);
            } else {
                dragWindow.moveTo(e.getX(), e.getY());
            }
        }
        
        public void mouseMoved(MouseEvent e){   }
        
        public void mouseReleased(MouseEvent e){ 
            if(visible){
                Point loc = dragWindow.getLocationOnScreen();
                TabFrame target = TabUtils.getTabFrameAtLocation(loc);
                visible = false;
                //System.err.println("mouseReleased @ " + target);
                if(target == null){
                    TabUtils.launchTab(TabUtils.getTabFrame(TabControl.this), TabControl.this.getIndex(), loc);
                } else if(owner.id == target.id){
                    Point p = e.getLocationOnScreen();
                    int x = p.x - owner.getX();
                    int y = p.y - owner.getY();
                    Component hit = owner.findComponentAt(x, y);
                    if((hit != null) && (hit instanceof TabInsertionHandler)){
                        //tab rearranging
                        TabInsertionHandler handler = (TabInsertionHandler)hit;
                        handler.handleInsertion(getIndex());
                    }
                } else {
                    TabUtils.transferTab(TabUtils.getTabFrame(TabControl.this), TabControl.this.getIndex(), target);
                }
                dragWindow.setVisible(false);
                dragWindow.dispose();
                dragWindow = null;
            }
            
        }
        
        public void mouseEntered(MouseEvent e){ 
           TabControl.this.setCursor(hand);
        }
        
        public void mouseExited(MouseEvent e){ 
            TabControl.this.setCursor(cursor);
        }
        
    }

    //highlights the buttons upon mouse enters
    private final static MouseListener buttonMouseListener = new MouseAdapter(){
            public void mouseEntered(MouseEvent e) {
                Component component = e.getComponent();
                if (component instanceof AbstractButton) {
                    AbstractButton button = (AbstractButton) component;
                    button.setBorderPainted(true);
                }
            }
        
            public void mouseExited(MouseEvent e) {
                Component component = e.getComponent();
                if (component instanceof AbstractButton) {
                    AbstractButton button = (AbstractButton) component;
                    button.setBorderPainted(false);
                }
            }
        };

}


class DragWindow extends Window {
    Point offset; 
    
    DragWindow(Window w, Point loc){
        super(w);
        offset = loc;
        setMinimumSize(new Dimension(20, 20));
    }

    public void moveTo(int x, int y){
        setLocation(x + offset.x, y + offset.y);

    }
    
    public Insets getInsets(){ return new Insets(1,1,1,1); }

}



