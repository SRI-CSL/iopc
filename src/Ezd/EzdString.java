/*
 * See copyright notice in this directory!
 */
package Ezd;

import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Rectangle;

public class EzdString extends EzdGraphic {

    final static int NONE = 0;              // Used to define text position in
    public final static int NORTH = 1;      //  bounding box.
    public final static int NORTHWEST = 2;
    public final static int WEST = 3;
    public final static int SOUTHWEST = 4;
    public final static int SOUTH = 5;
    public final static int SOUTHEAST = 6;
    public final static int EAST = 7;
    public final static int NORTHEAST = 8;
    public final static int CENTER = 9;

    private String text;
    private int x;          // Coordinates of base line or bounding box.
    private int y;
    private int xPixel;     // Coordinates in pixels.
    private int yPixel;
    private int width;      // Optional bounding box.
    private int height;
    private int widthPixel;
    private int heightPixel;
    private int position;   // Position in bounding box.
    private Color color;
    private Font font;
    private int ascent;
    private int descent;
    private int advance;

    public EzdString( String text, int x, int y, Color color, Font font, FontMetrics fm) {
        initEzdString( text, x, y, 0, 0, NONE, color, font, fm);
    }

    public EzdString( String text, int x, int y, int width, int height,
                      int position, Color color, Font font, FontMetrics fm ) {
        initEzdString( text, x, y, width, height, position, color, font, fm );
    }

    void initEzdString( String text, int x, int y, int width, int height,
                        int position, Color color, Font font, FontMetrics fm ) {
        if (width < 0)
            throw new IllegalArgumentException( "Width must be >= 0" );
        if (height < 0)
            throw new IllegalArgumentException( "Height must be >= 0" );
        this.text = text;
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
        this.position = position;
        this.color = color;
        this.font = font;
        ascent = fm.getMaxAscent();
        descent = fm.getMaxDescent();
        advance = fm.stringWidth( text );
    }

    void move( int dx, int dy )  {
        x = x+dx;
        y = y+dy;
    }

    void paint( Graphics g, EzdMap m ) {
        if (color == null) return;
        g.setColor( color );
        g.setFont( font );
        ToPixel( m );
        g.drawString( text, xPixel, yPixel );
    }

    Rectangle boundingRect( EzdMap m ) {
        ToPixel( m );
        return new Rectangle( xPixel, yPixel-ascent, advance, ascent+descent );
    }

    boolean intersects( EzdMap m, Rectangle r ) {
        ToPixel( m );
        return r.intersects( new Rectangle( xPixel, yPixel-ascent,
                                            advance, ascent+descent ) );
    }

    private void ToPixel( EzdMap m ) {
        xPixel = Math.min( m.xToPixel( x ), m.xToPixel( x+width-1 ) );
        yPixel = Math.min( m.yToPixel( y ), m.yToPixel( y+height-1 ) );
        widthPixel = m.widthToPixel( width );
        heightPixel = m.heightToPixel( height );
        switch (position) {
            case NORTHWEST: case WEST: SOUTHWEST:
                break;
            case NORTH: case CENTER: case SOUTH:
                xPixel = xPixel+(widthPixel-advance)/2;
                break;
            case NORTHEAST: case EAST: case SOUTHEAST:
                xPixel = xPixel+widthPixel-advance;
                break;
        }
        switch (position) {
            case NORTHWEST: case NORTH: case NORTHEAST:
                yPixel = yPixel+ascent;
                break;
            case WEST: case CENTER: case EAST:
                yPixel = yPixel+(heightPixel-ascent-descent)/2+ascent;
                break;
            case SOUTHWEST: case SOUTH: case SOUTHEAST:
                yPixel = yPixel+heightPixel-descent;
                break;
        }
    }
}
