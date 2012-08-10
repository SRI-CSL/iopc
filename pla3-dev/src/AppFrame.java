import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

//the kb manager is the black sheep here ...
public class AppFrame extends JFrame { 
    
    private static int counter = 0;
    public final int id = counter++;
    
    //may as well build in jlambda access from the start

    public final JMenuBar globalMenuBar  = new JMenuBar();
    protected final JMenu windowsMenu = new JMenu("Windows");


    static {
        if("Mac OS X".equals(System.getProperty("os.name"))){
            System.setProperty("apple.laf.useScreenMenuBar","true");
        }
    }

    public AppFrame(String title){
        globalMenuBar.add(windowsMenu);
        this.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        this.setTitle(title + " " + id);
        this.setJMenuBar(globalMenuBar);
        TabUtils.register(this);
    }
    

    public void dispose(){
        TabUtils.unregister(this);
        super.dispose();
    }

    public boolean containsLocationOnScreen(Point p){
        return contains(p.x - getX(), p.y - getY());
    }
    
}

