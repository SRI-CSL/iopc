/*
 * See copyright notice in this directory!
 */
package Ezd;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Rectangle;


public class EzdFillRectangle extends EzdGraphic {
    private int x;
    private int y;
    private int width;
    private int height;
    private Color color;

    public EzdFillRectangle( int x, int y, int width, int height, Color color ) {
        if (width < 0)
            throw new IllegalArgumentException( "Width must be >= 0" );
        if (height < 0)
            throw new IllegalArgumentException( "Height must be >= 0" );
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
        this.color = color;
    }

    void move( int dx, int dy )  {
        x = x+dx;
        y = y+dy;
    }

    void paint( Graphics g, EzdMap m ) {
        if (color == null) return;
        g.setColor( color );
        g.fillRect( Math.min( m.xToPixel( x ), m.xToPixel( x+width ) ),
                    Math.min( m.yToPixel( y ), m.yToPixel( y+height ) ),
                    m.widthToPixel( width ), m.heightToPixel( height ) );
    }

    Rectangle boundingRect( EzdMap m ) {
        return new Rectangle( Math.min( m.xToPixel( x ), m.xToPixel( x+width ) ),
                              Math.min( m.yToPixel( y ), m.yToPixel( y+height ) ),
                              m.widthToPixel( width )+1,
                              m.heightToPixel( height )+1 );
    }

    boolean intersects( EzdMap m, Rectangle r ) {
        Rectangle self = new Rectangle( x, y, width, height );
        return r.intersects( self );
    }
}
