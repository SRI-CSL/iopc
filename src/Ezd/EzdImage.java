/*
 * See copyright notice in this directory!
 */

package Ezd;

import java.awt.Rectangle;
import java.awt.Graphics;
import java.awt.Image;

import java.awt.image.ImageObserver;
import java.awt.image.PixelGrabber;

public class EzdImage extends EzdGraphic implements ImageObserver {

    private Image image;
    private int x;
    private int y;
    private int pixelWidth;
    private int pixelHeight;

    public EzdImage( Image image, int x, int y ) {
        this.image = image;
        this.x = x;
        this.y = y;
        this.pixelWidth = image.getWidth( this );
        this.pixelHeight = image.getHeight( this );
        if (this.pixelWidth < 0)
            throw new IllegalArgumentException( "Width must be >= 0" );
        if (this.pixelHeight < 0)
            throw new IllegalArgumentException( "Height must be >= 0" );
    }

    void move( int dx, int dy )  {
        x = x+dx;
        y = y+dy;
    }

    void paint( Graphics g, EzdMap m ) {
        g.drawImage( image,
                     Math.min( m.xToPixel( x ),
                               m.xToPixel( x+m.pixelToWidth( pixelWidth ) ) ),
                     Math.min( m.yToPixel( y ),
                               m.yToPixel( y+m.pixelToHeight( pixelHeight ) ) ),
                     this );
    }

    public boolean imageUpdate( Image img, int flags, int x, int y, int width,
                                int height ) {
        return false;
    }

    Rectangle boundingRect( EzdMap m ) {
        return new Rectangle( Math.min( m.xToPixel( x ),
                                    m.xToPixel( x+m.pixelToWidth( pixelWidth ) ) ),
                              Math.min( m.yToPixel( y ),
                                    m.yToPixel( y+m.pixelToHeight( pixelHeight ) ) ),
                              pixelWidth+1, pixelHeight+1 );
    }

    boolean intersects( EzdMap m, Rectangle r ) {
        Rectangle intersect;
        intersect = r.intersection( new Rectangle( x, y,
                                               m.pixelToWidth( pixelWidth ),
                                               m.pixelToHeight( pixelHeight ) ) );
        if (intersect.width > 0  &&  intersect.height > 0) {
            int  px, py, pw, ph;
            int[]  pixels;
            PixelGrabber  pg;
            px = Math.min( m.xToPixel( intersect.x-x ),
                           m.xToPixel( intersect.x-x+intersect.width ) )-m.originX;
            py = Math.min( m.yToPixel( intersect.y-y ),
                           m.yToPixel( intersect.y-y+intersect.height ) )-m.originY;
            pw = m.widthToPixel( intersect.width );
            ph = m.heightToPixel( intersect.height );
            pixels = new int[ pw*ph ];
            pg = new PixelGrabber( image, px, py, pw, ph, pixels, 0, pw );
            try  {
                if (pg.grabPixels( 100 ) == false) return true;
            } catch (InterruptedException  e) {
                return false;
            }
            if  ((pg.status()  &  ImageObserver.ABORT)  !=  0)  {
                return false;
            }
            for (int i = 0; i < pixels.length; i++) {
                if (((pixels[i]>>24) & 0xff) != 0) return true;
            }
        }
        return false;
    }
}
