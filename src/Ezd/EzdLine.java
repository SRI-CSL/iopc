/*
 * See copyright notice in this directory!
 */
package Ezd;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Rectangle;


public class EzdLine extends EzdGraphic {
    private int x1;
    private int y1;
    private int x2;
    private int y2;
    private Color color;

    public EzdLine( int x1, int y1, int x2, int y2, Color color ) {
        this.x1 = x1;
        this.y1 = y1;
        this.x2 = x2;
        this.y2 = y2;
        this.color = color;
    }

    void move( int dx, int dy )  {
        x1 = x1+dx;
        y1 = y1+dy;
        x2 = x2+dx;
        y2 = y2+dy;
    }

    void paint( Graphics g, EzdMap m ) {
        if (color == null) return;
        g.setColor( color );
        g.drawLine( m.xToPixel( x1 ), m.yToPixel( y1 ),
                    m.xToPixel( x2 ), m.yToPixel( y2 ) );
    }

    Rectangle boundingRect( EzdMap m ) {
        int px1 = m.xToPixel( x1 );
        int px2 = m.xToPixel( x2 );
        int py1 = m.yToPixel( y1 );
        int py2 = m.yToPixel( y2 );

        return new Rectangle( Math.min( px1, px2 )-1, Math.min( py1, py2 )-1,
                              Math.abs( px1-px2 )+2, Math.abs( py1-py2 )+2 );
    }

    boolean intersects( EzdMap m, Rectangle r ) {
        if (r.contains( x1, y1 ) || r.contains( x2, y2 )) return true;
        if ((sequence( x1, r.x, x2 ) &&
             sequence( r.y, xToY( r.x ), r.y+r.height)) ||
            (sequence( x1, r.x+r.width, x2 ) &&
             sequence( r.y, xToY( r.x+r.width), r.y+r.height )))
            return true;
        if ((sequence( y1, r.y, y2 ) &&
             sequence( r.x, yToX( r.y ), r.x+r.width)) ||
            (sequence( y1, r.y+r.height, y2 ) &&
             sequence( r.x, yToX( r.y+r.height), r.x+r.width )))
            return true;
        return false;
    }

    private boolean sequence( int a, int b, int c ) {
        return (a <= b && b <= c && a < c) || (c <= b && b <= a && c < a);
    }

    private int xToY( int x ) {
        return (int)((x-x1)*((double)(y2-y1)/(x2-x1))+y1);
    }

    private int yToX( int y ) {
        return (int)((x2-x1)*((double)(y-y1)/(y2-y1))+x1);
    }

}
