package GraphAct;

import java.awt.Point;
import java.awt.Color;
import java.awt.FontMetrics;
import java.awt.Frame;

import Ezd.EzdArc;
import Ezd.EzdFillArc;
import Ezd.EzdString;


class Circle extends Thing {

    private static final int width = 60;
    private final FontMetrics fm = frame.getFontMetrics(textBold);
    private final int nameWidth = fm.stringWidth(name);
    private final int height = fm.getHeight();

    Circle(Node node, Frame frame){
	super(node, frame, width);
	draw(false);
    }

    public Point getCenter(){
	return new Point(this.x + width/2, this.y + width/4);
    }

    public void draw( boolean outline ) {
        if (outline) {
	    //	    if(nameWidth > (3 * width)/ 2)
	    drawAs(new EzdFillArc(x - nameWidth/2, y, width + nameWidth, width/2  + height, 0, 360, Color.white ),
		   new EzdFillArc(x - nameWidth/2, y, width + nameWidth, width/2  + height, 0, 360, color ),
		   new EzdArc( x - nameWidth/2, y, width + nameWidth, width/2  + height, 0, 360, Color.black ),
		   new EzdString(name, x - nameWidth/2, y, width + nameWidth, width/2  + height, EzdString.CENTER, Color.black,  textBold, fm));
	    //	    else 
	    //	drawAs(new EzdFillArc(x, y, width, width/2, 0, 360, color ),
	    //	       new EzdArc(x, y, width, width/2, 0, 360, Color.black ),
	    //	       new EzdString(name, x, y, width, width/2, EzdString.CENTER, Color.black,  textBold, fm));

        } else {
	    FontMetrics fm = frame.getFontMetrics(textPlain);
            drawAs( new EzdFillArc( x, y, width, width/2, 0, 360, Color.white ),
		    new EzdFillArc( x, y, width, width/2, 0, 360, color ),
		    new EzdString(name, x, y, width, width/2, EzdString.CENTER, Color.black,  textPlain, fm)
		    );
        }
    }

}
