package GraphAct;

import java.awt.FontMetrics;
import java.awt.Color;
import java.awt.Frame;
import java.awt.Point;

import Ezd.EzdString;
import Ezd.EzdFillRectangle;
import Ezd.EzdRectangle;

class Rectangle extends Thing {

    private static final int width = 40;

    Rectangle(Node node, Frame frame){
	super(node, frame, width);
	draw(false);
    }

    public Point getCenter(){
	return new Point(this.x + width/2, this.y + width/4);
    }

    public void draw( boolean outline ) {
        if (outline) {
	    FontMetrics fm = frame.getFontMetrics(textBold);
            drawAs( new EzdFillRectangle( x, y, width, width/2, Color.white ),
                    new EzdFillRectangle( x, y, width, width/2, color ),
                    new EzdRectangle( x, y, width, width/2, Color.black ),
		    new EzdString(name, x, y, width, width/2, EzdString.CENTER, Color.black,  textBold, fm));
        } else {
	    FontMetrics fm = frame.getFontMetrics(textPlain);
            drawAs( new EzdFillRectangle( x, y, width, width/2, Color.white ),
		    new EzdFillRectangle( x, y, width, width/2, color ),
		    new EzdString(name, x, y, width, width/2, EzdString.CENTER, Color.black,  textPlain, fm));
        }
    }

}
