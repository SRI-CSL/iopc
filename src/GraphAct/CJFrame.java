package GraphAct;

import java.awt.Point;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.GraphicsEnvironment;
import javax.swing.JFrame;

public class CJFrame extends JFrame {
    public static final float WHOLE          = 1;
    public static final float THREEQUARTERS  = 1.33333F;
    public static final float TWOTHIRDS      = 1.5F;
    public static final float HALF           = 2;
    public static final float THIRD          = 3;
    public static final float QUARTER        = 4;
    static private int xPos; 
    static private int yPos;

     CJFrame(){
	 this("", THREEQUARTERS);
     }

     CJFrame(String title){
	 this(title, THREEQUARTERS);
     }

     CJFrame(String title, float ratio){
         super(title);
         Dimension d = Toolkit.getDefaultToolkit().getScreenSize();
         int screenH = d.height;
         int screenW = d.width;
         setSize((int)(screenW/ratio), (int)(screenH/ratio));
	 if(xPos == 0){ xPos = (int)(screenW * (ratio - 1)/(2 * ratio)); } else { xPos += 10; }
	 if(yPos == 0){ yPos = (int)(screenW * (ratio - 1)/(2 * ratio)); } else { yPos += 10; }
         setLocation(xPos, yPos);
     }

     CJFrame(String title, int width, int height){
         super(title);
         setSize(width, height);
	 if(xPos == 0){ xPos = width  / 4 ; } else { xPos += 10; }
	 if(yPos == 0){ yPos = height / 4 ; } else { yPos += 10; }
         setLocation(xPos, yPos);
     }

     CJFrame(String title, int width, int height, int x, int y){
         super(title);
         setSize(width, height);
         setLocation(x, y);
     }

    public Point getRandomPoint(){
	Dimension d = getSize();
	return new Point((int)((d.width - 50) * Math.random()),
			 (int)((d.height - 50) * Math.random()));
    }
    
    public Point getRandomPoint(int y){
	Dimension d = getSize();
	return new Point((int)((d.width - 50) * Math.random()), y);
    }

    public Component add(Component c){
	this.getContentPane().add(c);
	return c;
    }

     public static void main(String[] args) {
         CJFrame f = new CJFrame("CJ Frame", THREEQUARTERS);
         f.setVisible(true);
	 GraphicsEnvironment g = GraphicsEnvironment.getLocalGraphicsEnvironment();
	 String[] fonts = g.getAvailableFontFamilyNames();
	 for(int i = 0; i < fonts.length; i++)
	     IO.err.println(fonts[i]);
     }
     
     
 }

