/*
 * See copyright notice in this directory!
 */
package Ezd;

class EzdMap {

    EzdDrawing drawing;     // drawing
    int originX;            // Origin of drawing on EzdView
    int originY;
    double scaleX;          // Scale X and Y coordinates to EzdView
    double scaleY;

    EzdMap( EzdDrawing d, int x, int y, double scaleX, double scaleY ) {
        drawing = d;
        originX = x;
        originY = y;
        this.scaleX = scaleX;
        this.scaleY = scaleY;
    }

    int xToPixel( int x ) {
        return (int)(x*scaleX+originX);
    }

    int pixelToX( int pixel ) {
        return (int)((pixel-originX)/scaleX);
    }

    int widthToPixel( int w ) {
        return (int)Math.abs(w*scaleX);
    }

    int pixelToWidth( int w ) {
        return (int)Math.abs(w/scaleX);
    }

    int yToPixel( int y ) {
        return (int)(y*scaleY+originY);
    }

    int pixelToY( int pixel ) {
        return (int)((pixel-originY)/scaleY);
    }

    int heightToPixel( int h ) {
        return (int)Math.abs(h*scaleY);
    }

    int pixelToHeight( int h ) {
        return (int)Math.abs(h/scaleY);
    }

    int startToPixel( int start ) {
        if (scaleX >= 0.0) {
            if (scaleY >= 0.0)
                return -start;
            else
                return start;
        } else {
            if (scaleY >= 0.0)
                return start+180;
            else
                return 180-start;
        }
    }

    int spanToPixel( int span ) {
        if (scaleX >= 0.0) {
            if (scaleY >= 0.0)
                return -span;
            else
                return span;
        } else {
            if (scaleY >= 0.0)
                return span;
            else
                return -span;
        }
    }

}
