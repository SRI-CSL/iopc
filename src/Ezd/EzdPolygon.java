/*
 * See copyright notice in this directory!
 */

package Ezd;

import java.awt.Polygon;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Rectangle;

public class EzdPolygon extends EzdGraphic {

    private Polygon polygon;
    private Color color;
    private int x1, y1, x2, y2;

    public EzdPolygon( Polygon polygon, Color color ) {
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
        g.drawPolygon( new Polygon( x, y, polygon.npoints ) );
    }

    Rectangle boundingRect( EzdMap m ) {
        Rectangle bb = polygon.getBounds();

        return new Rectangle( Math.min( m.xToPixel( bb.x ),
                                        m.xToPixel( bb.x+bb.width ) )-1,
                              Math.min( m.yToPixel( bb.y ),
                                        m.yToPixel( bb.y+bb.height )-1 ),
                              m.widthToPixel( bb.width )+2,
                              m.heightToPixel( bb.height )+2 );
    }

    boolean intersects( EzdMap m, Rectangle r ) {
        for (int i = 0; i < polygon.npoints-1; i++) {
            x1 = polygon.xpoints[ i ];
            y1 = polygon.ypoints[ i ];
            x2 = polygon.xpoints[ i+1 ];
            y2 = polygon.ypoints[ i+1 ];
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
        }
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

