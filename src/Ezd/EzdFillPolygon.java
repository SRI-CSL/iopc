/*
 * See copyright notice in this directory!
 */
package Ezd;

import java.awt.Polygon;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Rectangle;


public class EzdFillPolygon extends EzdGraphic {

    private Polygon polygon;
    private Color color;

    public EzdFillPolygon( Polygon polygon, Color color ) {
        this.polygon = new Polygon( polygon.xpoints, polygon.ypoints,
                                    polygon.npoints );
        this.color = color;
    }

    void move( int dx, int dy )  {
        for (int i = 0; i < polygon.npoints; i++) {
            polygon.xpoints[ i ] = polygon.xpoints[ i ]+dx;
            polygon.ypoints[ i ] = polygon.ypoints[ i ]+dy;
        }
    }

    void paint( Graphics g, EzdMap m ) {
        int x[] = new int[ polygon.npoints ],
            y[] = new int[ polygon.npoints ];
        if (color == null) return;
        g.setColor( color );
        for (int i = 0; i < polygon.npoints; i++) {
            x[ i ] = m.xToPixel( polygon.xpoints[ i ] );
            y[ i ] = m.yToPixel( polygon.ypoints[ i ] );
        }
        g.fillPolygon( new Polygon( x, y, polygon.npoints ) );
    }

    Rectangle boundingRect( EzdMap m ) {
        Rectangle bb = polygon.getBounds();
        return new Rectangle( Math.min( m.xToPixel( bb.x ),
                                        m.xToPixel( bb.x+bb.width ) )-1,
                              Math.min( m.yToPixel( bb.y ),
                                        m.yToPixel( bb.y+bb.height ) )-1,
                              m.widthToPixel( bb.width )+2,
                              m.heightToPixel( bb.height )+2 );
    }

    boolean intersects( EzdMap m, Rectangle r ) {
        return( polygon.contains( r.x, r.y ) ||
                polygon.contains( r.x+r.width, r.y ) ||
                polygon.contains( r.x, r.y+r.height ) ||
                polygon.contains( r.x+r.width, r.y+r.height ) );
    }
}
