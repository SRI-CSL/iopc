/*
 * See copyright notice in this directory!
 */

package Ezd;

import java.util.ArrayList;
import java.util.ListIterator;

import java.awt.Graphics;
import java.awt.Rectangle;

public class EzdDrawing {

    ArrayList glyphs;          // ArrayList containing the glyphs in the drawing.
    ArrayList views;           // ArrayList contains the views displaying the drawing

    public final static int BOTTOM = 0;
    public final static int TOP = 2147483647;

    public EzdDrawing() {
        glyphs = new ArrayList();
        views = new ArrayList();
        Ezd.version();
    }

    public void add( EzdGlyph g ) {
        synchronized( Ezd.mutex ) {
            if (g.drawing == this) {
		int gindex = glyphs.indexOf(g);
		if(gindex >= 0) glyphs.remove(gindex);
            } else {
                g.setDrawing( this );
            }
            glyphs.add( g );
            damaged( g, false );
        }
    }

    public void add( EzdGraphic graphic ) {
        EzdGlyph g = new EzdGlyph();
        g.drawAs( graphic );
        add( g );
    }

    public void insertAt( EzdGlyph g, int index ) {
        synchronized( Ezd.mutex ) {
            if (g.drawing == this) {
		int gindex = glyphs.indexOf(g);
		if(gindex >= 0) glyphs.remove(gindex);
            } else {
                g.setDrawing( this );
            }
            index = Math.min( Math.max( 0, index ), glyphs.size() );
            glyphs.add(index, g);
            damaged( g, false );
        }
    }

    public int indexOf( EzdGlyph g ) {
        synchronized( Ezd.mutex ) {
            return glyphs.indexOf( g );
        }
    }

    public void remove( EzdGlyph g ) {
        EzdView v;

        synchronized( Ezd.mutex ) {
            if  (g.drawing == null) return;
            damaged( g, false );
	    int gindex = glyphs.indexOf(g);
	    if(gindex >= 0) glyphs.remove(gindex);
            g.setDrawing( null );
	    ListIterator iter =  views.listIterator();
	    while(iter.hasNext()){
		v = (EzdView)iter.next();
                if (g == v.getCurrentGlyph()) v.setCurrentGlyph( null );
                if (g == v.getMouseGrab()) v.setMouseGrab( null );
                if (g == v.getKeyboardGrab()) v.setKeyboardGrab( null );
	    }
        }
    }

    public void removeAll() {
        EzdGlyph g;
        EzdView v;
        synchronized( Ezd.mutex ) {
            glyphs = new ArrayList();
            damaged( null, false );
	    ListIterator iter = views.listIterator();
	    while(iter.hasNext()){
		v = (EzdView)iter.next();
                if ((g = v.getCurrentGlyph()) != null  &&  g.drawing == this) {
                    v.setCurrentGlyph( null );
                }
                if ((g = v.getMouseGrab()) != null  &&  g.drawing == this) {
                    v.setMouseGrab( null );
                }
                if ((g = v.getKeyboardGrab()) != null  &&  g.drawing == this) {
                    v.setKeyboardGrab( null );
                }
	    }
        }
    }

    void damaged( EzdGlyph g, boolean defer ) {
        EzdView  v;
        Rectangle r;
        boolean rInUpdate;
	ListIterator iter = views.listIterator();
	while(iter.hasNext()){
	    v = (EzdView)iter.next();
            if (g == null)
                r = v.updateRect();
            else
                r = g.boundingRect( v.drawingMap( this ) );
            rInUpdate = r.intersects( v.updateRect() );
            if (rInUpdate  ||  v.deferredDamage != null) {
                if (v.deferredDamage == null)
                    v.deferredDamage = r;
                else if (rInUpdate)
                    v.deferredDamage.add( r );
                if (defer == false) {
                    if (v == Ezd.mouseView  ||
                        v.deferredDamage.contains( Ezd.mouseX, Ezd.mouseY ))
                        Ezd.mouseDamaged = true;
                    if (Ezd.insideHandler())
                        Ezd.deferRepaint( v );
                    else {
                        v.repaint( v.deferredDamage.x, v.deferredDamage.y,
                                   v.deferredDamage.width, v.deferredDamage.height );
                        v.deferredDamage = null;
                    }
                }
            }
	}
    }
    /*
    void paint( Graphics g, EzdMap m ) {
        EzdGlyph glyph;
        Rectangle clip;
        clip = g.getClipBounds();
	ListIterator iter = glyphs.listIterator();
	while(iter.hasNext()){
	    glyph = (EzdGlyph)iter.next();
            if (clip.intersects(glyph.boundingRect(m))) glyph.paint(g, m);
	}
    }

    */

    void paint( Graphics g, EzdMap m ) {
        EzdGlyph glyph;
	//        Rectangle clip;
	//        clip = g.getClipBounds();
	ListIterator iter = glyphs.listIterator();
	while(iter.hasNext()){
	    glyph = (EzdGlyph)iter.next();
	    //            if (clip.intersects(glyph.boundingRect(m)))
	    glyph.paint(g, m);
	}
    }


}
