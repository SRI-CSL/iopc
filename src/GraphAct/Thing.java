package GraphAct;

import java.awt.Color;
import java.awt.Font;
import java.awt.Frame;
import java.awt.Point;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import java.util.ListIterator;

import Ezd.EzdDrawing;
import Ezd.EzdGlyph;
import Ezd.EzdView;

/*
Bitstream Charter
Courier
Courier 10 Pitch
Cursor
Default
Dialog
DialogInput
Lucida Bright
Lucida Sans
Lucida Sans Typewriter
Luxi Mono
Luxi Sans
Luxi Serif
Monospaced
SansSerif
Serif
Utopia
*/

abstract class Thing extends EzdGlyph {
    private boolean DEBUG = false;
    protected String name;
    protected Frame frame;
    protected Color  color;
    protected int id;
    protected int  x, y;
    protected Node node;
    protected int mx, my;
    protected int w;
    static protected Font textPlain = new Font( "Lucinda Sans", Font.PLAIN, 9 );
    static protected Font textBold = new Font( "Lucinda Sans", Font.BOLD, 12 );
    protected ArrayList lines = new ArrayList();
    protected final String onClick;

    Thing(Node node, Frame frame, int w){
	this.node = node;
	if(node.level < 0) 
	    this.x = 65 * this.node.xRank;
	else 
	    this.x = 50 * this.node.xRank;
        this.y = 50 * this.node.yRank;
	this.name = node.label;
        this.w = w;
	this.color = node.color;
	this.frame = frame;
	this.onClick = node.onclick;
    }

    public String toString(){ return name; }

    public int getId(){ return this.id; }

    public void reRank(){
	this.x = 65 * this.node.xRank;
        this.y = 50 * this.node.yRank;
    }

    public void addLine(Line line){
	this.lines.add(line);
    }
    
    public void drawLines(){
	ListIterator iter = this.lines.listIterator();
	while(iter.hasNext())((Line)iter.next()).draw();
    }

    abstract public Point getCenter();
    abstract public void draw( boolean outline );

    public void mousePressed( EzdView v, MouseEvent me, int x, int y ) {
        getDrawing().insertAt( this, EzdDrawing.TOP );
	this.mx = x;
	this.my = y;
	if(me.isShiftDown()){
	    ActorMsg.sendActorMsg(IO.out, onClick);
	    if(DEBUG) ActorMsg.sendActorMsg(IO.err, onClick);
	} else {
	    if(DEBUG) ActorMsg.sendActorMsg(IO.err, onClick);
	}
    }
    
    public void mouseEntered( EzdView v, MouseEvent me, int x, int y ) {
 	this.mx = x;
	this.my = y;
	//      brings the glyph to the front
	getDrawing().insertAt( this, EzdDrawing.TOP );
	draw(true);
    }

    public void mouseDragged( EzdView v, MouseEvent me, int x, int y ) {
	//	if(v.contains(x,y)){
	    //	    int deltaX = mx - x, deltaY = my - y;
	    //	    mx = x;
	    //	    my = y;
	    //	    this.x -= deltaX;
	    //	    this.y -= deltaY;
	    this.x = x - (w/2);
	    this.y = y - (w/4);
	    drawLines();
	    draw(true);

	    //	}
    }

    public void mouseExited( EzdView v, MouseEvent me, int x, int y ) {
        draw(false);
    }
}
