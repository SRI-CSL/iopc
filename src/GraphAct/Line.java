package GraphAct;

import java.awt.Color;
import java.awt.Frame;
import java.awt.Polygon;
import java.awt.Point;
import java.awt.FontMetrics;

import Ezd.EzdGlyph;
import Ezd.EzdFillPolygon;
import Ezd.EzdLine;
import Ezd.EzdString;

class Line extends EzdGlyph {
    protected Frame frame;
    protected Edge edge;
    protected Thing thingA;
    protected Thing thingB;
    protected Color color = Color.black;
    protected String label = "d";

    /*
    Line(Thing thingA, Thing thingB, Frame frame) {
	this.thingA = thingA;
	this.thingB = thingB;
	this.frame = frame;
	thingA.addLine(this);
	thingB.addLine(this);
	draw();
    }
    */
    
    Line(Edge edge, Thing thingA, Thing thingB, Frame frame) {
	this.edge = edge;
	this.thingA = thingA;
	this.thingB = thingB;
	this.frame = frame;
	this.color = edge.color;
	if(edge.label != null) this.label = edge.label;
	thingA.addLine(this);
	thingB.addLine(this);
	draw();
    }

    public String toString(){ return  "line " + label; }

    void draw() {
	FontMetrics fm = frame.getFontMetrics(Thing.textPlain);
	Polygon p;
	float gradient;
	Point centerA = thingA.getCenter();
	Point centerB = thingB.getCenter();
	int midX = centerA.x + (centerB.x - centerA.x)/2;
	int midY = centerA.y + (centerB.y - centerA.y)/2;
	p = new Polygon();
	p.addPoint(midX - 5, midY - 5);
	p.addPoint(midX + 5, midY - 5);
	p.addPoint(midX + 5, midY + 5);
	p.addPoint(midX - 5, midY + 5);
	drawAs(
	       new EzdFillPolygon(p, color), // Color.black), 
	       new EzdLine(centerA.x, centerA.y, centerB.x, centerB.y, color),
	       new EzdString(this.label, midX - 5, midY - 5, 10, 10, EzdString.CENTER, Color.black, Thing.textPlain, fm));
	//	IO.err.println(xyToAngle(centerB.x - centerA.x, centerB.y - centerA.y));
    }

    double xyToAngle( int x, int y ) {
        double angle;
        angle = Math.atan2( x, y ) -  Math.PI/2;
	if (angle <=  - Math.PI ) angle = angle + (2*Math.PI);
        return angle;
    }

}

