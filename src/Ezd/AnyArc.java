/*
 * See copyright notice in this directory!
 */

package Ezd;

import java.awt.Color;
import java.awt.Rectangle;

abstract class AnyArc extends EzdGraphic {

    int x;
    int y;
    int width;
    int height;
    int start;
    int span;
    Color color;
    float centerX;
    float centerY;

    AnyArc( int x, int y, int width, int height,
            int start, int span, Color color ) {
        double fociDistance;

        if (width < 0)
            throw new IllegalArgumentException( "Width must be >= 0" );
        if (height < 0)
            throw new IllegalArgumentException( "Height must be >= 0" );
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
        if (span < 0) {
            start = start+span;
            span = -span;
        }
        start = start%360;
        if (start < 0) start = start+360;
        if (span > 360) span = 360;
        this.start = start;
        this.span = span;
        this.color = color;
        centerX = x+((float)width)/2;
        centerY = y+((float)height)/2;
    }

    void move( int dx, int dy )  {
        x = x+dx;
        y = y+dy;
        centerX = centerX+dx;
        centerY = centerY+dy;
    }

    Rectangle boundingRect( EzdMap m ) {
        return new Rectangle( Math.min( m.xToPixel( x ), m.xToPixel( x+width ) ),
                              Math.min( m.yToPixel( y ), m.yToPixel( y+height ) ),
                              m.widthToPixel( width )+1,
                              m.heightToPixel( height )+1 );
    }

    boolean inArcAngle( Rectangle r ) {
        return inArcAngle( r.x, r.y ) ||
               inArcAngle( r.x, r.y+r.width ) ||
               inArcAngle( r.x+r.width, r.y ) ||
               inArcAngle( r.x+r.width, r.y+r.height ) ||
               inArcAngle( r.x+r.width/2, r.y+r.height/2 );
    }

    boolean inArcAngle( float x, float y ) {
        double angle;
        angle = (Math.atan2( y-centerY, x-centerX ))*(180/Math.PI);
        if (angle < 0) angle = angle+360;
        if (start+span > 360)
            return start+span-360 >= angle  ||  angle >= start;
        else
            return start <= angle  &&  angle <= start+span;
    }

    int inRadius( Rectangle r ) {
        int flags;
        flags = inRadius( r.x, r.y )+inRadius( r.x, r.y+r.width )+
                inRadius( r.x+r.width, r.y )+inRadius( r.x+r.width, r.y+r.width )+
                inRadius( r.x+r.width/2, r.y+r.height/2 );
        if (flags == -5)
            return -1;
        else if (flags == 5)
            return 1;
        else
            return 0;
    }

    int inRadius( int x, int y ) {
        float distance;
        distance = (x-centerX)*(x-centerX)/(width*width/4)+
                   (y-centerY)*(y-centerY)/(height*height/4);
        if (distance < 1) return -1;
        if (distance == 1) return 0;
        return 1;
    }
}
