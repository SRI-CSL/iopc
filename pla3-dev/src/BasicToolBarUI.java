import javax.swing.plaf.basic.*;

import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;

import java.beans.*;

import java.util.Hashtable;
import java.util.HashMap;

import javax.swing.border.*;
import javax.swing.plaf.*;

public class BasicToolBarUI extends ToolBarUI implements SwingConstants {
    protected JToolBar toolBar;

    private boolean floating;

    private int floatingX;
    private int floatingY;

    private JFrame floatingFrame;
    private RootPaneContainer floatingToolBar;
    protected DragWindow dragWindow;
    private Container dockingSource;
    private int dockingSensitivity = 0;
    protected int focusedCompIndex = -1;

    
    protected final Color dockingColor = Color.white;
    protected final Color floatingColor = Color.white;
    protected final Color dockingBorderColor = Color.green;
    protected final Color floatingBorderColor = Color.red;

    protected MouseInputListener dockingListener;
    protected PropertyChangeListener propertyListener;

    protected ContainerListener toolBarContListener;
    protected FocusListener toolBarFocusListener;
    private Handler handler;

    protected String constraintBeforeFloating = BorderLayout.NORTH;
    
    //ComponentUI
    public static ComponentUI createUI(JComponent c){   return new BasicToolBarUI();  }

    //ComponentUI
    public void installUI(JComponent c){
        toolBar = (JToolBar) c;
        LookAndFeel.installBorder(toolBar,"ToolBar.border");    
        LookAndFeel.installColorsAndFont(toolBar, "ToolBar.background", "ToolBar.foreground", "ToolBar.font");
        installListeners();
        dockingSensitivity = 0;
        floating = false;
        floatingX = floatingY = 0;
        floatingToolBar = null;
        setOrientation(toolBar.getOrientation());
        LookAndFeel.installProperty(c, "opaque", Boolean.TRUE);
    }
    
    //ComponentUI
    public void uninstallUI(JComponent c){
        uninstallListeners();
        if(isFloating() == true){  setFloating(false, null);  }
        floatingToolBar = null;
        dragWindow = null;
        dockingSource = null;
    }

    protected void installListeners(){
        dockingListener = createDockingListener();
        
        if(dockingListener != null){
            toolBar.addMouseMotionListener(dockingListener);
            toolBar.addMouseListener(dockingListener);
        }

        propertyListener = createPropertyListener(); 
        if(propertyListener != null){
            toolBar.addPropertyChangeListener(propertyListener);
        }

        toolBarContListener = createToolBarContListener();
        if(toolBarContListener != null){
            toolBar.addContainerListener(toolBarContListener);
        }

        toolBarFocusListener = createToolBarFocusListener();

        if(toolBarFocusListener != null){
            Component[] components = toolBar.getComponents();
            for (int i = 0; i < components.length; ++i){
                components[ i ].addFocusListener(toolBarFocusListener);
            }
        }
    }

    protected void uninstallListeners(){
        if(dockingListener != null){
            toolBar.removeMouseMotionListener(dockingListener);
            toolBar.removeMouseListener(dockingListener);
            dockingListener = null;
        }

        if(propertyListener != null){
            toolBar.removePropertyChangeListener(propertyListener);
            propertyListener = null;  // removed in setFloating
        }

        if(toolBarContListener != null){
            toolBar.removeContainerListener(toolBarContListener);
            toolBarContListener = null;
        }

        if(toolBarFocusListener != null){
            // Remove focus listener from all components in toolbar
            Component[] components = toolBar.getComponents();

            for (int i = 0; i < components.length; ++i){
                components[ i ].removeFocusListener(toolBarFocusListener);
            }

            toolBarFocusListener = null;
        }
        handler = null;
    }


    
    protected RootPaneContainer createFloatingWindow(JToolBar toolbar){
        class ToolBarDialog extends JDialog {
            public ToolBarDialog(Frame owner, String title, boolean modal){ super(owner, title, modal);  }

            public ToolBarDialog(Dialog owner, String title, boolean modal){ super(owner, title, modal); }

            // Override createRootPane() to automatically resize the frame when contents change
            protected JRootPane createRootPane(){
                JRootPane rootPane = new JRootPane(){
                        private boolean packing = false;

                        public void validate(){
                            super.validate();
                            if(!packing){
                                packing = true;
                                pack();
                                packing = false;
                            }
                        }
                    };
                rootPane.setOpaque(true);
                return rootPane;
            }
        }

        JDialog dialog;
        Window window = SwingUtilities.getWindowAncestor(toolbar);
        if(window instanceof Frame){
            dialog = new ToolBarDialog((Frame)window, toolbar.getName(), false);
        } else if(window instanceof Dialog){
            dialog = new ToolBarDialog((Dialog)window, toolbar.getName(), false);
        } else {
            dialog = new ToolBarDialog((Frame)null, toolbar.getName(), false);
        }

        dialog.getRootPane().setName("ToolBar.FloatingWindow");
        dialog.setTitle(toolbar.getName());
        dialog.setResizable(false);
        WindowListener wl = createFrameListener();
        dialog.addWindowListener(wl);
        return dialog;
    }

    protected DragWindow createDragWindow(JToolBar toolbar){
        Window frame = null;
        if(toolBar != null){
            Container p;
            for(p = toolBar.getParent() ; p != null && !(p instanceof Window) ; p = p.getParent());
            if(p != null && p instanceof Window){ frame = (Window) p; }
        }
        if(floatingToolBar == null){ floatingToolBar = createFloatingWindow(toolBar); }
        if(floatingToolBar instanceof Window) frame = (Window) floatingToolBar;
        DragWindow dragWindow = new DragWindow(frame);
        return dragWindow;
    }


    public void setFloatingLocation(int x, int y){
        floatingX = x;
        floatingY = y;
    }
    
    public boolean isFloating(){ return floating; }

    public void setFloating(boolean b, Point p){
        if(toolBar.isFloatable() == true){
            boolean visible = false;
            Window ancestor = SwingUtilities.getWindowAncestor(toolBar);
            if(ancestor != null){ visible = ancestor.isVisible(); }
            if(dragWindow != null){ dragWindow.setVisible(false); }
            this.floating = b;
            if(floatingToolBar == null){
                floatingToolBar = createFloatingWindow(toolBar);
            }
            if(b == true){
                if(dockingSource == null){
                    dockingSource = toolBar.getParent();
                    dockingSource.remove(toolBar);
                }
                constraintBeforeFloating = calculateConstraint();
                if(propertyListener != null){ UIManager.addPropertyChangeListener(propertyListener); }
                floatingToolBar.getContentPane().add(toolBar,BorderLayout.CENTER);
                if(floatingToolBar instanceof Window){
                    ((Window)floatingToolBar).pack();
                    ((Window)floatingToolBar).setLocation(floatingX, floatingY);
                    if(visible){
                        ((Window)floatingToolBar).setVisible(true);
                    } else {
                        ancestor.addWindowListener(new WindowAdapter(){ 
                                public void windowOpened(WindowEvent e){
                                    ((Window)floatingToolBar).setVisible(true);
                                }
                            });
                    }
                }
            } else {
                if(floatingToolBar == null){  floatingToolBar = createFloatingWindow(toolBar); }
                if(floatingToolBar instanceof Window) ((Window)floatingToolBar).setVisible(false);
                floatingToolBar.getContentPane().remove(toolBar);
                String constraint = getDockingConstraint(dockingSource, p);
                if(constraint == null){ constraint = BorderLayout.NORTH; }
                int orientation = mapConstraintToOrientation(constraint);
                setOrientation(orientation);
                if(dockingSource== null){ dockingSource = toolBar.getParent(); }
                if(propertyListener != null){  UIManager.removePropertyChangeListener(propertyListener); }
                dockingSource.add(constraint, toolBar);
            }
            dockingSource.invalidate();
            Container dockingSourceParent = dockingSource.getParent();
            if(dockingSourceParent != null){  dockingSourceParent.validate(); }
            dockingSource.repaint();
        }
    }

    private int mapConstraintToOrientation(String constraint){
        int orientation = toolBar.getOrientation();

        if(constraint != null){
            if(constraint.equals(BorderLayout.EAST) || constraint.equals(BorderLayout.WEST)){ orientation = JToolBar.VERTICAL; }
            else if(constraint.equals(BorderLayout.NORTH) || constraint.equals(BorderLayout.SOUTH)){ orientation = JToolBar.HORIZONTAL; }
        }

        return orientation;
    }
    
    public void setOrientation(int orientation){   
        toolBar.setOrientation(orientation);
        if(dragWindow != null){ dragWindow.setOrientation(orientation); }
    }
    
    
    private boolean isBlocked(Component comp, Object constraint){
        if(comp instanceof Container){
            Container cont = (Container)comp;
            LayoutManager lm = cont.getLayout();
            if(lm instanceof BorderLayout){
                BorderLayout blm = (BorderLayout)lm;
                Component c = blm.getLayoutComponent(cont, constraint);
                return (c != null && c != toolBar);
            }
        }
        return false;
    }

    public boolean canDock(Component c, Point p){
        return (p != null && getDockingConstraint(c, p) != null);
    }

    private String calculateConstraint(){
        String constraint = null;
        LayoutManager lm = dockingSource.getLayout();
        if(lm instanceof BorderLayout){
            constraint = (String)((BorderLayout)lm).getConstraints(toolBar);
        }
        return (constraint != null) ? constraint : constraintBeforeFloating;
    }



    private String getDockingConstraint(Component c, Point p){
        if(p == null) return constraintBeforeFloating;
        if(c.contains(p)){
            dockingSensitivity = (toolBar.getOrientation() == JToolBar.HORIZONTAL)
                ? toolBar.getSize().height
                : toolBar.getSize().width;
            // North  (Base distance on height for now!)
            if(p.y < dockingSensitivity && !isBlocked(c, BorderLayout.NORTH)){
                return BorderLayout.NORTH;
            }
            // East  (Base distance on height for now!)
            if(p.x >= c.getWidth() - dockingSensitivity && !isBlocked(c, BorderLayout.EAST)){
                return BorderLayout.EAST;
            }
            // West  (Base distance on height for now!)
            if(p.x < dockingSensitivity && !isBlocked(c, BorderLayout.WEST)){
                return BorderLayout.WEST;
            }
            if(p.y >= c.getHeight() - dockingSensitivity && !isBlocked(c, BorderLayout.SOUTH)){
                return BorderLayout.SOUTH;
            }
        }
        return null;
    }

    protected void dragTo(Point position, Point origin){
        if(toolBar.isFloatable() == true){
            try {
                if(dragWindow == null)
                    dragWindow = createDragWindow(toolBar);
                Point offset = dragWindow.getOffset();
                if(offset == null){
                    Dimension size = toolBar.getPreferredSize();
                    offset = new Point(size.width/2, size.height/2);
                    dragWindow.setOffset(offset);
                }
                Point global = new Point(origin.x+ position.x,
                                         origin.y+position.y);
                Point dragPoint = new Point(global.x- offset.x, 
                                            global.y- offset.y);
                if(dockingSource == null)
                    dockingSource = toolBar.getParent();
                constraintBeforeFloating = calculateConstraint();       
                Point dockingPosition = dockingSource.getLocationOnScreen();
                Point comparisonPoint = new Point(global.x-dockingPosition.x,
                                                  global.y-dockingPosition.y);
                if(canDock(dockingSource, comparisonPoint)){
                    dragWindow.setBackground(dockingColor);    
                    String constraint = getDockingConstraint(dockingSource,
                                                             comparisonPoint);
                    int orientation = mapConstraintToOrientation(constraint);
                    dragWindow.setOrientation(orientation);
                    dragWindow.setBorderColor(dockingBorderColor);
                } else {
                    dragWindow.setBackground(floatingColor);
                    dragWindow.setBorderColor(floatingBorderColor);
                }
                
                dragWindow.setLocation(dragPoint.x, dragPoint.y);
                if(dragWindow.isVisible() == false){
                    Dimension size = toolBar.getPreferredSize();
                    dragWindow.setSize(size.width, size.height);
                    dragWindow.setVisible(true);
                }
            }
            catch (IllegalComponentStateException e){}
        }
    }

    protected void floatAt(Point position, Point origin){
        if(toolBar.isFloatable() == true){
            try {
                Point offset = dragWindow.getOffset();
                if(offset == null){
                    offset = position;
                    dragWindow.setOffset(offset);
                }
                Point global = new Point(origin.x+ position.x,
                                         origin.y+position.y);
                setFloatingLocation(global.x-offset.x, 
                                    global.y-offset.y);
                if(dockingSource != null){ 
                    Point dockingPosition = dockingSource.getLocationOnScreen();
                    Point comparisonPoint = new Point(global.x-dockingPosition.x,
                                                      global.y-dockingPosition.y);
                    if(canDock(dockingSource, comparisonPoint)){
                        setFloating(false, comparisonPoint);
                    } else {
                        setFloating(true, null);
                    }
                } else {
                    setFloating(true, null);
                }
                dragWindow.setOffset(null);
            }
            catch (IllegalComponentStateException e) {}
        }
    }

    private Handler getHandler(){
        if(handler == null){ handler = new Handler();  }
        return handler;
    }

    protected ContainerListener createToolBarContListener(){  return getHandler(); }

    protected FocusListener createToolBarFocusListener(){ return getHandler();  }

    protected PropertyChangeListener createPropertyListener(){ return getHandler(); }

    protected MouseInputListener createDockingListener(){
        getHandler().tb = toolBar;
        return getHandler();
    }
    
    protected WindowListener createFrameListener(){
        return new FrameListener();
    }
    
    protected void paintDragWindow(Graphics g){
        g.setColor(dragWindow.getBackground());     
        int w = dragWindow.getWidth();
        int h = dragWindow.getHeight();
        g.fillRect(0, 0, w, h);
        g.setColor(dragWindow.getBorderColor());
        g.drawRect(0, 0, w - 1, h - 1);     
    }
    
    private class Handler implements ContainerListener, FocusListener, MouseInputListener, PropertyChangeListener {
        
        // ContainerListener
        
        public void componentAdded(ContainerEvent evt){
            Component c = evt.getChild();

            if(toolBarFocusListener != null){
                c.addFocusListener(toolBarFocusListener);
            }

        }

        public void componentRemoved(ContainerEvent evt){
            Component c = evt.getChild();

            if(toolBarFocusListener != null){
                c.removeFocusListener(toolBarFocusListener);
            }

        }


        // FocusListener

        public void focusGained(FocusEvent evt){
            Component c = evt.getComponent();
            focusedCompIndex = toolBar.getComponentIndex(c);
        }

        public void focusLost(FocusEvent evt){ }



        // MouseInputListener (DockingListener)

        JToolBar tb;
        boolean isDragging = false;
        Point origin = null;

        public void mousePressed(MouseEvent evt){ 
            if(!tb.isEnabled()){
                return;
            }
            isDragging = false;
        }

        public void mouseReleased(MouseEvent evt){
            if(!tb.isEnabled()){
                return;
            }
            if(isDragging == true){
                Point position = evt.getPoint();
                if(origin == null)
                    origin = evt.getComponent().getLocationOnScreen();
                floatAt(position, origin);
            }
            origin = null;
            isDragging = false;
        }

        public void mouseDragged(MouseEvent evt){
            if(!tb.isEnabled()){
                return;
            }
            isDragging = true;
            Point position = evt.getPoint();
            if(origin == null){
                origin = evt.getComponent().getLocationOnScreen();
            }
            dragTo(position, origin);
        }

        public void mouseClicked(MouseEvent evt){}
        public void mouseEntered(MouseEvent evt){}
        public void mouseExited(MouseEvent evt){}
        public void mouseMoved(MouseEvent evt){}



        // PropertyChangeListener

        public void propertyChange(PropertyChangeEvent evt){
            String propertyName = evt.getPropertyName();
            if(propertyName == "lookAndFeel"){
                toolBar.updateUI();
            } else if(propertyName == "orientation"){
                // Search for JSeparator components and change it's orientation
                // to match the toolbar and flip it's orientation.
                Component[] components = toolBar.getComponents();
                int orientation = ((Integer)evt.getNewValue()).intValue();
                JToolBar.Separator separator;

                for (int i = 0; i < components.length; ++i){
                    if(components[i] instanceof JToolBar.Separator){
                        separator = (JToolBar.Separator)components[i];
                        if((orientation == JToolBar.HORIZONTAL)){
                            separator.setOrientation(JSeparator.VERTICAL);
                        } else {
                            separator.setOrientation(JSeparator.HORIZONTAL);
                        }
                        Dimension size = separator.getSeparatorSize();
                        if(size != null && size.width != size.height){
                            // Flip the orientation.
                            Dimension newSize =
                                new Dimension(size.height, size.width);
                            separator.setSeparatorSize(newSize);
                        }
                    }
                }
            } 
        }
    }

    protected class FrameListener extends WindowAdapter {
        public void windowClosing(WindowEvent w){      
            if(toolBar.isFloatable() == true){
                if(dragWindow != null)
                    dragWindow.setVisible(false);
                floating = false;
                if(floatingToolBar == null)
                    floatingToolBar = createFloatingWindow(toolBar);
                if(floatingToolBar instanceof Window) ((Window)floatingToolBar).setVisible(false);
                floatingToolBar.getContentPane().remove(toolBar);
                String constraint = constraintBeforeFloating;
                if(toolBar.getOrientation() == JToolBar.HORIZONTAL){
                    if(constraint == "West" || constraint == "East"){ constraint = "North"; }
                } else {
                    if(constraint == "North" || constraint == "South"){ constraint = "West"; }
                }
                if(dockingSource == null){ dockingSource = toolBar.getParent(); }
                if(propertyListener != null){ UIManager.removePropertyChangeListener(propertyListener); }
                dockingSource.add(toolBar, constraint);
                dockingSource.invalidate();
                Container dockingSourceParent = dockingSource.getParent();
                if(dockingSourceParent != null){ dockingSourceParent.validate(); }
                dockingSource.repaint();
            }
        }

    } 



    protected class DragWindow extends Window {
        Color borderColor = Color.gray;
        int orientation = toolBar.getOrientation();
        Point offset; // offset of the mouse cursor inside the DragWindow

        DragWindow(Window w){ super(w); }
    
        public int getOrientation(){  return orientation; }

        public void setOrientation(int o){
            if(isShowing()){
                if(o == this.orientation)
                    return;     
                this.orientation = o;
                Dimension size = getSize();
                setSize(new Dimension(size.height, size.width));
                if(offset!=null){
                    if(o == JToolBar.HORIZONTAL){
                        setOffset(new Point(size.height-offset.y, offset.x));
                    } else {
                        setOffset(new Point(offset.y, size.width-offset.x));
                    }
                }
                repaint();
            }
        }

        public Point getOffset(){ return offset;  }

        public void setOffset(Point p){  this.offset = p; }
    
        public void setBorderColor(Color c){
            if(this.borderColor == c)
                return;
            this.borderColor = c;
            repaint();
        }

        public Color getBorderColor(){ return this.borderColor; }

        public void paint(Graphics g){
            paintDragWindow(g);
            // Paint the children
            super.paint(g);
        }
        public Insets getInsets(){ return new Insets(1,1,1,1); }
    }
}