/*
 * See copyright notice in this directory!
 */
package Ezd;

import java.awt.event.MouseEvent;
import java.awt.event.KeyEvent;

import java.awt.Rectangle;
import java.awt.Graphics;


public class EzdGlyph {

    EzdDrawing drawing;                 // Drawing containing the Glyph
    private EzdGraphic[] graphics;      // Graphics describing the object
    private EzdMap cachedMap;           // Used to cache a boundingRect.
    private double scale;                  // Used to store the cacheMap's scale
    private Rectangle cachedRect;

    public final void drawAs() {
        drawAs( new EzdGraphic[0] );
    }

    public final void drawAs( EzdGraphic g1 ) {
        EzdGraphic[] g = { g1 };
        drawAs( g );
    }

    public final void drawAs( EzdGraphic g1, EzdGraphic g2 ) {
        EzdGraphic[] g = { g1, g2 };
        drawAs( g );
    }

    public final void drawAs( EzdGraphic g1, EzdGraphic g2, EzdGraphic g3 ) {
        EzdGraphic[] g = { g1, g2, g3 };
        drawAs( g );
    }

    public final void drawAs( EzdGraphic g1, EzdGraphic g2, EzdGraphic g3,
                              EzdGraphic g4 ) {
        EzdGraphic[] g = { g1, g2, g3, g4 };
        drawAs( g );
    }

    public final void drawAs( EzdGraphic g1, EzdGraphic g2, EzdGraphic g3,
                              EzdGraphic g4, EzdGraphic g5 ) {
        EzdGraphic[] g = { g1, g2, g3, g4, g5 };
        drawAs( g );
    }

    public final void drawAs( EzdGraphic g1, EzdGraphic g2, EzdGraphic g3,
                              EzdGraphic g4, EzdGraphic g5, EzdGraphic g6 ) {
        EzdGraphic[] g = { g1, g2, g3, g4, g5, g6 };
        drawAs( g );
    }

    public final void drawAs( EzdGraphic g1, EzdGraphic g2, EzdGraphic g3,
                              EzdGraphic g4, EzdGraphic g5, EzdGraphic g6,
                              EzdGraphic g7 ) {
        EzdGraphic[] g = { g1, g2, g3, g4, g5, g6, g7 };
        drawAs( g );
    }

    public final void drawAs( EzdGraphic g1, EzdGraphic g2, EzdGraphic g3,
                              EzdGraphic g4, EzdGraphic g5, EzdGraphic g6,
                              EzdGraphic g7, EzdGraphic g8 ) {
        EzdGraphic[] g = { g1, g2, g3, g4, g5, g6, g7, g8 };
        drawAs( g );
    }

    public final void drawAs( EzdGraphic g1, EzdGraphic g2, EzdGraphic g3,
                              EzdGraphic g4, EzdGraphic g5, EzdGraphic g6,
                              EzdGraphic g7, EzdGraphic g8, EzdGraphic g9 ) {
        EzdGraphic[] g = { g1, g2, g3, g4, g5, g6, g7, g8, g9 };
        drawAs( g );
    }

    public final void drawAs( EzdGraphic g1, EzdGraphic g2, EzdGraphic g3,
                              EzdGraphic g4, EzdGraphic g5, EzdGraphic g6,
                              EzdGraphic g7, EzdGraphic g8, EzdGraphic g9,
                              EzdGraphic g10 ) {
        EzdGraphic[] g = { g1, g2, g3, g4, g5, g6, g7, g8, g9, g10 };
        drawAs( g );
    }

    public final void drawAs( EzdGraphic g1, EzdGraphic g2, EzdGraphic g3,
                              EzdGraphic g4, EzdGraphic g5, EzdGraphic g6,
                              EzdGraphic g7, EzdGraphic g8, EzdGraphic g9,
                              EzdGraphic g10, EzdGraphic g11 ) {
        EzdGraphic[] g = { g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11 };
        drawAs( g );
    }

    public final void drawAs( EzdGraphic g1, EzdGraphic g2, EzdGraphic g3,
                              EzdGraphic g4, EzdGraphic g5, EzdGraphic g6,
                              EzdGraphic g7, EzdGraphic g8, EzdGraphic g9,
                              EzdGraphic g10, EzdGraphic g11, EzdGraphic g12 ) {
        EzdGraphic[] g = { g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11, g12 };
        drawAs( g );
    }

    public final void drawAs( EzdGraphic g1, EzdGraphic g2, EzdGraphic g3,
                              EzdGraphic g4, EzdGraphic g5, EzdGraphic g6,
                              EzdGraphic g7, EzdGraphic g8, EzdGraphic g9,
                              EzdGraphic g10, EzdGraphic g11, EzdGraphic g12,
                              EzdGraphic g13 ) {
        EzdGraphic[] g = { g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11, g12, g13 };
        drawAs( g );
    }

    public final void drawAs( EzdGraphic g1, EzdGraphic g2, EzdGraphic g3,
                              EzdGraphic g4, EzdGraphic g5, EzdGraphic g6,
                              EzdGraphic g7, EzdGraphic g8, EzdGraphic g9,
                              EzdGraphic g10, EzdGraphic g11, EzdGraphic g12,
                              EzdGraphic g13, EzdGraphic g14 ) {
        EzdGraphic[] g = { g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11, g12, g13,
                        g14 };
        drawAs( g );
    }

    public final void drawAs( EzdGraphic g1, EzdGraphic g2, EzdGraphic g3,
                              EzdGraphic g4, EzdGraphic g5, EzdGraphic g6,
                              EzdGraphic g7, EzdGraphic g8, EzdGraphic g9,
                              EzdGraphic g10, EzdGraphic g11, EzdGraphic g12,
                              EzdGraphic g13, EzdGraphic g14, EzdGraphic g15 ) {
        EzdGraphic[] g = { g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11, g12, g13,
                        g14, g15 };
        drawAs( g );
    }

    public void drawAs( EzdGraphic[] g ) {
        if (drawing == null) {
            graphics = g;
        } else {
            synchronized( Ezd.mutex ) {
                drawing.damaged( this, true );
                graphics = g;
                cachedMap = null;
                drawing.damaged( this, false );
            }
        }
    }

    public void move( int dx, int dy ) {
        if (drawing == null) {
            for (int i = 0; i < graphics.length; i++) {
                graphics[i].move( dx, dy );
            }
        } else {
            synchronized( Ezd.mutex ) {
                drawing.damaged( this, true );
                for (int i = 0; i < graphics.length; i++) {
                    graphics[i].move( dx, dy );
                }
                cachedMap = null;
                drawing.damaged( this, false);
            }
        }
    }

    public EzdDrawing getDrawing() {
        return drawing;
    }

    final void setDrawing( EzdDrawing d ) {
        if (d != null  &&  drawing != null)
            throw new IllegalArgumentException( "EzdGlyph already in a drawing" );
        drawing = d;
        cachedMap = null;
        if (graphics == null) graphics = new EzdGraphic[0];
    }

    final void paint( Graphics g, EzdMap m ) {
        for (int i = 0; i < graphics.length; i++) {
            graphics[i].paint( g, m );
        }
    }

    final Rectangle boundingRect( EzdMap m ) {
	// zooming complexifies this
	if ((m == cachedMap) && (m.scaleX == scale)) return cachedRect;
	if (graphics.length == 0) return new Rectangle( -1, -1, 0, 0 );
        cachedMap = m;
	scale = m.scaleX;
        cachedRect = graphics[0].boundingRect( m );
        for (int i = 1; i < graphics.length; i++) {
            cachedRect.add( graphics[i].boundingRect( m ) );
        }
        return cachedRect;
    }

    final boolean intersects( EzdMap m, Rectangle r ) {
        for (int i = graphics.length-1; i >= 0; i--) {
            if (graphics[i].intersects( m, r )) return true;
        }
        return false;
    }

    public EzdGlyph underMouse() {
        return Ezd.mouseView.findGlyph( Ezd.mouseX, Ezd.mouseY, this, false);
    }

    public void keyPressed( EzdView v, KeyEvent e) {
    }

    public void keyReleased( EzdView v, KeyEvent e) {
    }

    public void keyTyped( EzdView v, KeyEvent e) {
    }

    public void mousePressed( EzdView v, MouseEvent e, int x, int y ) {
    }

    public void mouseReleased(EzdView v, MouseEvent e, int x, int y) {
    }

    public void mouseClicked(EzdView v, MouseEvent e, int x, int y) {
    }

    public void mouseEntered(EzdView v, MouseEvent e, int x, int y ) {
    }

    public void mouseExited(EzdView v, MouseEvent e, int x, int y ) {
    }

    public void mouseDragged(EzdView v, MouseEvent e, int x, int y) {
    }
    
    public void mouseMoved(EzdView v, MouseEvent e, int x, int y) {   
    }


}
