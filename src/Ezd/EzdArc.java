/*
 * See copyright notice in this directory!
 */

package Ezd;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Rectangle;

public class EzdArc extends AnyArc {

    public EzdArc( int x, int y, int width, int height,
                   int startAngle, int span, Color color ) {
        super( x, y, width, height, startAngle, span, color );
    }

    void paint( Graphics g, EzdMap m ) {
        if (color == null) return;
        g.setColor( color );
        g.drawArc( Math.min( m.xToPixel( x ), m.xToPixel( x+width ) ),
                   Math.min( m.yToPixel( y ), m.yToPixel( y+height ) ),
                   m.widthToPixel( width ), m.heightToPixel( height ),
                   m.startToPixel( start ), m.spanToPixel( span ) );
    }

    boolean intersects( EzdMap m, Rectangle r ) {
        return inRadius( r ) == 0  && inArcAngle( r );
    }
}
