/*
 * See copyright notice in this directory!
 */
package Ezd;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Rectangle;

public class EzdFillOval extends AnyArc {

    public EzdFillOval( int x, int y, int width, int height, Color color ) {
        super( x, y, width, height, 0, 360, color);
    }

    void paint( Graphics g, EzdMap m ) {
        if (color == null) return;
        g.setColor( color );
        g.fillOval( Math.min( m.xToPixel( x ), m.xToPixel( x+width ) ),
                    Math.min( m.yToPixel( y ), m.yToPixel( y+height ) ),
                    m.widthToPixel( width ), m.heightToPixel( height ) );
    }

    boolean intersects( EzdMap m, Rectangle r ) {
        return inRadius( r ) <= 0;
    }
}
