/*
 * See copyright notice in this directory!
 */
package Ezd;

import javax.swing.JComponent;
import javax.swing.Scrollable;
import javax.swing.SwingConstants;

import java.awt.Dimension;
import java.awt.Rectangle;
import java.awt.Graphics;
import java.awt.Color;

import java.awt.event.MouseEvent;
import java.awt.event.KeyEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.KeyListener;

import java.util.ArrayList;
import java.util.ListIterator;

public class EzdView extends JComponent 
    implements MouseListener, MouseMotionListener, KeyListener, Scrollable  {

    private double scale = 1.0;             // Scale factor (Ian's idea)
    private Dimension origsize;             // Origonal size


    private Dimension minsize;              // Minimum size
    private Dimension prefsize;             // Prefered size
    private Rectangle updateRect;           // Rectangle that's visible
    Rectangle deferredDamage;               // Damage that has not yet been repainted.
    private EzdGlyph mouseGrab;             // Glyph that has grabbed the mouse.
    private EzdGlyph keyboardGrab;          // Glyph that has grabbed the keyboard.
    private EzdGlyph currentGlyph;          // Glyph that mouse is in.
    private boolean hasFocus = true;        // Indicates view has UI focus.
    private ArrayList mappedDrawings;       // Set of EzdDrawings mapped onto this view
    public final static int BOTTOM = 0;
    public final static int TOP = 2147483647;
    public static final boolean DEBUG = false;

    /* Scrollable Interface                               */
    private int maxUnitIncrement = 1;

    public Dimension getPreferredScrollableViewportSize(){
	return prefsize;
    }
    public int getScrollableBlockIncrement(Rectangle visibleRect, int orientation, int direction){
        if (orientation == SwingConstants.HORIZONTAL) {
            return visibleRect.width - maxUnitIncrement;
        } else {
            return visibleRect.height - maxUnitIncrement;
        }
    } 
    public boolean getScrollableTracksViewportHeight(){ return false; }
    public boolean getScrollableTracksViewportWidth(){ return false; }
    public int getScrollableUnitIncrement(Rectangle visibleRect, int orientation, int direction){
        int currentPosition = 0;
        if (orientation == SwingConstants.HORIZONTAL) {
            currentPosition = visibleRect.x;
        } else {
            currentPosition = visibleRect.y;
        }
        if (direction < 0) {
            int newPosition = 
		currentPosition - (currentPosition / maxUnitIncrement) * maxUnitIncrement;
            return (newPosition == 0) ? maxUnitIncrement : newPosition;
        } else {
            return ((currentPosition / maxUnitIncrement) + 1) * maxUnitIncrement - currentPosition;
        }
    }

    /* KeyListener  Interface                               */
    public void keyPressed(KeyEvent ke){ 
        EzdGlyph g;
        synchronized(Ezd.mutex){
	    if(!hasFocus) return;
	    Ezd.enterHandler();
            g = findKeyboardGlyph(ke);
            if(g != null) g.keyPressed(this, ke);
            Ezd.exitHandler();
        }
    }

    public void keyReleased(KeyEvent ke){
        EzdGlyph g;
        synchronized(Ezd.mutex){
            if(!hasFocus) return;
            Ezd.enterHandler();
            g = findKeyboardGlyph(ke);
            if(g != null) g.keyReleased(this, ke);
            Ezd.exitHandler();
        }
    }

    public void keyTyped(KeyEvent ke){
        EzdGlyph g;
        synchronized(Ezd.mutex){
            if(!hasFocus) return;
            Ezd.enterHandler();
            g = findKeyboardGlyph(ke);
            if(g != null) g.keyTyped(this, ke);
            Ezd.exitHandler();
        }
    }

    /* MouseListener  Interface                               */
    public void mouseClicked(MouseEvent me){
        EzdGlyph g;
        EzdMap m;
        synchronized(Ezd.mutex){
	    hasFocus = requestFocusInWindow();
            if(!hasFocus) return;
            Ezd.enterHandler();
            g = findMouseGlyph(me, false);
            if(g != null){
                m = drawingMap(g.drawing);
                g.mouseClicked(this, me, m.pixelToX(me.getX()), m.pixelToY(me.getY()));
            }
            Ezd.exitHandler();
        }
 }

    public void mouseEntered(MouseEvent me){
        synchronized(Ezd.mutex){
            Ezd.enterHandler();
            findMouseGlyph(me, false);
            Ezd.exitHandler();
        }
    }


  public void mouseExited(MouseEvent me){ 
        EzdMap m;
        synchronized(Ezd.mutex){
            Ezd.enterHandler();
            findMouseGlyph(me, false);
            if(currentGlyph != null){
                m = drawingMap(currentGlyph.drawing);
                currentGlyph.mouseExited(this, me, m.pixelToX(me.getX()), m.pixelToY(me.getY()));
                currentGlyph = null;
            }
            Ezd.exitHandler();
        }
    }

    public void mousePressed(MouseEvent me){
        EzdGlyph g;
        EzdMap m;
        synchronized(Ezd.mutex){
            Ezd.enterHandler();
            g = findMouseGlyph(me, DEBUG);
            if(g != null){
                m = drawingMap(g.drawing);
                g.mousePressed(this, me, m.pixelToX(me.getX()), m.pixelToY(me.getY()));
            }
            Ezd.exitHandler();
        }
    }

    public void mouseReleased(MouseEvent me){
        EzdGlyph g;
        EzdMap m;
        synchronized(Ezd.mutex){
            if(!hasFocus) return;
            Ezd.enterHandler();
            g = findMouseGlyph(me, false);
            if(g != null){
                m = drawingMap(g.drawing);
                g.mouseReleased(this, me, m.pixelToX(me.getX()), m.pixelToY(me.getY()));
            }
            Ezd.exitHandler();
        }
    }

    /* MouseMotionListener  Interface                           */
    public void mouseDragged(MouseEvent me){ 
        EzdGlyph g;
        EzdMap m;
        synchronized(Ezd.mutex){
            Ezd.enterHandler();
            g = findMouseGlyph(me, false);
            if(g != null){
                m = drawingMap(g.drawing);
                g.mouseDragged(this, me, m.pixelToX(me.getX()), m.pixelToY(me.getY()));
            }
            Ezd.exitHandler();
        }
    }

    public void mouseMoved(MouseEvent me){
        EzdGlyph g;
        EzdMap m;
        synchronized(Ezd.mutex){
            Ezd.enterHandler();
            g = findMouseGlyph(me, false);
            if(g != null){
                m = drawingMap(g.drawing);
                g.mouseMoved(this, me, m.pixelToX(me.getX()), m.pixelToY(me.getY()));
            }
            Ezd.exitHandler();
        }
    }


    public EzdView(){
        this(0, 0);
    }

    public EzdView(int width, int height){
        super();
        minsize = new Dimension(width, height);
        prefsize = new Dimension(width, height);
        origsize = new Dimension(width, height);
        updateRect = new Rectangle(0, 0, width, height);
        mappedDrawings = new ArrayList();
	setAutoscrolls(true);
	this.addKeyListener(this);
	this.addMouseListener(this);
	this.addMouseMotionListener(this);
    }

    public void setBackground(Color color){
        super.setBackground(color);
        repaintAll();
    }

    public void add(EzdDrawing d){
        insertAt(d, mappedDrawings.size(), 0, 0, 1.0, 1.0);
    }

    public void insertAt(EzdDrawing d, int index){
        insertAt(d, index, 0, 0, 1.0, 1.0);
    }

    public void insertAt(EzdDrawing d, int i, int oX, int oY, double sX, double sY){
        synchronized(Ezd.mutex){
            remove(d, false);
            i = Math.min(Math.max(0, i), mappedDrawings.size());
            mappedDrawings.add(i, new EzdMap(d, oX, oY, sX, sY));
            d.views.add(this);
            repaintAll();
        }
    }

    public void scroll(int dx, int dy){
        synchronized(Ezd.mutex){
	    EzdMap map;
            if(mappedDrawings.size() == 0) return;
	    ListIterator iter = mappedDrawings.listIterator();
	    while(iter.hasNext()){
		map = (EzdMap)iter.next();
                map.originX = map.originX + dx;
                map.originY = map.originY + dy;
	    }
            if(currentGlyph != null) Ezd.mouseDamaged = true;
            repaintAll();
        }
    }


    // Ian's idea
    public void zoom(double factor){
        synchronized(Ezd.mutex){
	    EzdMap map;
	    scale = factor;
            if(mappedDrawings.size() == 0) return;
	    ListIterator iter = mappedDrawings.listIterator();
	    while(iter.hasNext()){
		map = (EzdMap)iter.next();
                map.scaleX = scale;
                map.scaleY = scale;
	    }
	    Dimension d = getOrigonalSize();
	    Dimension nd = new Dimension((int)(d.width * scale), (int)(d.height * scale));
	    setSize(nd);
	    //	    System.err.println("width = " + nd.width + " height = " + nd.height);
            if(currentGlyph != null) Ezd.mouseDamaged = true;
            repaintAll();
        }
    }

    public void setMouseGrab(EzdGlyph mouse){
        mouseGrab = mouse;
        if(mouse == null  &&  Ezd.mouseView == this)
            repaint(Ezd.mouseX, Ezd.mouseY, 1, 1);
    }

    public EzdGlyph getMouseGrab(){
        return mouseGrab;
    }

    public void setKeyboardGrab(EzdGlyph keyboard){
        keyboardGrab = keyboard;
    }

    public EzdGlyph getKeyboardGrab(){
        return keyboardGrab;
    }

    public EzdGlyph getCurrentGlyph(){
        return currentGlyph;
    }

    void setCurrentGlyph(EzdGlyph g){
        currentGlyph = g;
    }

    public int indexOf(EzdDrawing d){
        synchronized(Ezd.mutex){
	    ListIterator iter = mappedDrawings.listIterator();
	    int index = -1;
	    while(iter.hasNext()){
		index++;
		EzdMap e = (EzdMap)iter.next();
		if(e.drawing == d) return index;
	    }
        }
        return  -1;
    }

    public void remove(EzdDrawing d){
        remove(d, true);
    }

    public void removeAll(){
	mappedDrawings.removeAll(mappedDrawings);
    }

    void remove(EzdDrawing d, boolean unGrab){
        EzdMap  m;
        synchronized(Ezd.mutex){
            m = drawingMap(d);
            if(m == null) return;
	    int mindex = mappedDrawings.indexOf(m);
	    if(mindex >= 0) mappedDrawings.remove(mindex);
	    int tindex = d.views.indexOf(this);
	    if(tindex >= 0) d.views.remove(tindex);
            if(currentGlyph != null){
                if(currentGlyph.drawing == d){
		    //  currentGlyph.mouseExit(this, null, 0, 0);
                    currentGlyph = null;
                }
                Ezd.mouseDamaged = true;
            }
            if(unGrab){
                if(mouseGrab != null  &&  mouseGrab.drawing == d){
                    mouseGrab = null;
                }
                if(keyboardGrab != null  &&  keyboardGrab.drawing == d){
                    keyboardGrab = null;
                }
            }
            repaintAll();
        }
    }

    void repaintAll(){
        if(Ezd.insideHandler()){
            deferredDamage = 
		new Rectangle(updateRect.x, updateRect.y, updateRect.width, updateRect.height);
            Ezd.deferRepaint(this);
        } else {
            repaint();
        }
    }

    EzdMap drawingMap(EzdDrawing d){
	ListIterator iter = mappedDrawings.listIterator();
	while(iter.hasNext()){
	    EzdMap m = (EzdMap)iter.next();
	    if(m.drawing == d) return m;
	}
        return null;
    }

    public Dimension getMinimumSize(){
	return minsize;
    }

    public Dimension getPreferredSize(){
        return prefsize;
    }

    public Dimension getOrigonalSize(){
        return origsize;
    }

    public void setOrigonalSize(Dimension d){
	origsize = d;
    }

    Rectangle updateRect(){
        return updateRect;
    }

    public void setSize(int width, int height){
	minsize = new Dimension(width, height);
	//	System.err.println("minsize = " + minsize);
        prefsize = new Dimension(width, height);
	//	System.err.println("prefsize = " + prefsize);
        updateRect = new Rectangle(0, 0, width, height);
	//	System.err.println("updateRect = " + updateRect);
        super.setSize(width, height);
    }


    public void setSize(Dimension d){
        setSize(d.width, d.height);
    }

    public void setBounds(int x, int y, int width, int height){
	minsize = new Dimension(width, height);
        prefsize = new Dimension(width, height);
        updateRect = new Rectangle(0, 0, width, height);
        super.setBounds(x, y, width, height);
    }


    public void update(Graphics g){
        Dimension d;
        Rectangle clip;
        EzdMap m;
        Graphics offGraphic;

        synchronized(Ezd.mutex){
            clip = g.getClipBounds();
	    //	    System.err.println("clip = " + clip);
            d = new Dimension(Math.max(clip.x+clip.width, 1), Math.max(clip.y+clip.height, 1));
            if(Ezd.offImage == null || d.width > Ezd.offDimension.width ||
                d.height > Ezd.offDimension.height){
                Ezd.offDimension = d;
                Ezd.offImage = createImage(d.width, d.height);
            }
            offGraphic = Ezd.offImage.getGraphics();
            offGraphic.clipRect(clip.x, clip.y, clip.width, clip.height);
            offGraphic.setColor(getBackground());
            offGraphic.fillRect(0, 0, d.width, d.height);

	    ListIterator iter = mappedDrawings.listIterator();
	    while(iter.hasNext()){
		m = (EzdMap)iter.next();
		m.drawing.paint(offGraphic, m);
	    }
            g.drawImage(Ezd.offImage, 0, 0, this);
        }
    }

    /*

    public void update(Graphics g){
	ListIterator iter = mappedDrawings.listIterator();
	while(iter.hasNext()){
	    EzdMap m = (EzdMap)iter.next();
	    m.drawing.paint(g, m);
	}
    }
    */


    public void paintComponent(Graphics g){
        update(g);
    }

    EzdGlyph findGlyph(int x, int y, EzdGlyph ignore, boolean debug){
        EzdMap m;
        EzdDrawing d;
        int minx, miny, minw, minh;
        Rectangle drawingCursor;
        Rectangle pixelCursor;
        EzdGlyph g;

	pixelCursor = new Rectangle(x-1, y-1, 3, 3);
	//        pixelCursor = new Rectangle((int)((x-1)/scale), (int)((y-1)/scale), 3, 3);
	if(debug)System.err.println("pixelCursor = " + pixelCursor);

	ListIterator iterM = mappedDrawings.listIterator(mappedDrawings.size());
	while(iterM.hasPrevious()){
	    m = (EzdMap)iterM.previous();
            d = m.drawing;
	    /*
            minx = (int)(Math.min(m.pixelToX(x-1), m.pixelToX(x+1)) * scale);
            miny = (int)(Math.min(m.pixelToY(y-1), m.pixelToY(y+1)) * scale);
	    minw = Math.max(m.pixelToWidth((int)(3 * scale)),  3);
	    minh = Math.max(m.pixelToHeight((int)(3 * scale)), 3);
            drawingCursor = new Rectangle(minx, miny, minw, minh);
	    */

            minx = Math.min(m.pixelToX(x-1), m.pixelToX(x+1));
            miny = Math.min(m.pixelToY(y-1), m.pixelToY(y+1));
	    minw = 3;
	    minh = 3;
            drawingCursor = new Rectangle(minx, miny, minw, minh);

	    if(debug)System.err.println("drawingCursor = " + drawingCursor);
	    ListIterator iterG = d.glyphs.listIterator(d.glyphs.size());
	    while(iterG.hasPrevious()){
		g = (EzdGlyph)iterG.previous();
		if(debug)System.err.println("" + g + " at " +  g.boundingRect(m));
		if(pixelCursor.intersects(g.boundingRect(m))){
		    if(debug)System.err.println("\trectangles intersect");
		    if(g.intersects(m, drawingCursor)){
			if(debug)System.err.println("\tg intersects (m, drawingCursor)");
			if(g != ignore){
			    if(debug)System.err.println("\tg != ignore");
			    return g;
			} else {
			    if(debug)System.err.println("\tg == ignore");
			}
		    } else {
			if(debug)System.err.println("\tg *doesn't* intersect (m, drawingCursor)");
		    }
		} else {
		    if(debug)System.err.println("\trectangles *don't* intersect");
		}
		/*
                if(pixelCursor.intersects(g.boundingRect(m)) &&
		   g.intersects(m, drawingCursor)  &&
		   g != ignore)
                    return g;
		*/
	    }
	}
        return null;
    }

    EzdGlyph findKeyboardGlyph(KeyEvent evt){
        EzdGlyph g;
        g = keyboardGrab;
        if(g != null) return g;
        return currentGlyph;
    }

    EzdGlyph findMouseGlyph(MouseEvent me, boolean debug){
	int x = me.getX(), y = me.getY();
        EzdGlyph g;
        EzdMap m;
        g = mouseGrab;
        if(g != null){
            Ezd.setMouse(this, me);
            return g;
        }
        g = findGlyph(x, y, null, debug);
	if(debug)System.err.println("findGlyph at " + x + " " + y + " returned " + g);
        if(g != currentGlyph){
            if(currentGlyph != null){
                m = drawingMap(currentGlyph.drawing);
                currentGlyph.mouseExited(this, me, m.pixelToX(x), m.pixelToY(y));
            }
            Ezd.setMouse(this, me);
            currentGlyph = g;
            if(g != null){
                m = drawingMap(g.drawing);
                g.mouseEntered(this, me, m.pixelToX(x), m.pixelToY(y));
            }
        }
        return g;
    }

}
