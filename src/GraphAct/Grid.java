package GraphAct;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.BorderLayout;
import java.awt.event.KeyEvent;
import java.awt.event.KeyAdapter;
import javax.swing.JScrollPane;
import java.util.ArrayList;
import java.util.ListIterator;

import Ezd.EzdView;
import Ezd.EzdDrawing;
import Ezd.EzdGraphic;
import Ezd.EzdGlyph;
import Ezd.EzdLine;

public class Grid extends GraphActController implements GraphActWindowThing { 

    private final EzdView  view = new EzdView();
    private final EzdDrawing  drawing = new EzdDrawing();
    private final CJFrame frame = new CJFrame();
    private final JScrollPane scrollPane  = new JScrollPane(view);

    private String label = "Test Grid";
    private int rows, width = 50, totalWidth;
    private int columns, height = 50, totalHeight;
    private Mesh mesh;
    private Rover rover;

    public void   setRows(int rows){  this.rows = rows; }
    public void   setColumns(int columns){  this.columns = columns; }
    public int    getWidth(){  return width; }
    public void   setWidth(int width){  this.width = width; }
    public void   setTotalWidth(){  this.totalWidth = columns * width; }
    public int    getHeight(){  return height; }
    public void   setHeight(int height){  this.height = height; }
    public void   setTotalHeight(){  this.totalHeight =  rows  * height; }
    public String getGraphActLabel(){ return label; }
    public void   setGraphActLabel(String label){ this.label = label; }
    public void   setViewSize(){ this.view.setSize(new Dimension(totalWidth, totalHeight)); }
    public void   setFrameSize(){ this.frame.setSize(new Dimension(totalWidth, totalHeight)); }
    public void   add(Object o){ this.addPeon(o); }

    static String testString =
	"(grid (label grid123) (rows 10) (columns 10) (contents (obstacle ob1 3 3 ) (rover rover 1 1 E E)))";

    /*
      (grid 
      (label name)
      (rows r)
      (columns c)
      (contents (obstacle id r c ) .... (rover id rR cR rDir aDir))
      )
    */

    public Grid(){
	this(testString);
    }

    public Grid(String string){
	Sexp list;
	try{
	    list = new Sexp(string);
	    Parser.parseGrid(this, list);
	}catch(ParseError pe){ IO.err.println(pe); }
    }

    public Grid(int rows, int columns, int width, int height){
	this.rows = rows;
	this.columns = columns;
	this.width = width;
	this.height = height;
	totalWidth  = columns * width;
	totalHeight = rows * height;
	Dimension size = new Dimension(totalWidth, totalHeight);
	view.setSize(size);
	frame.setSize(size);
	System.err.println("setting frame size to " + size);
	addPeon(new Obstacle(this, new Location(0, 0)));
	addPeon(new Obstacle(this, new Location(2, 2)));
	addPeon(new Obstacle(this, new Location(2, 3)));
	addPeon(new Obstacle(this, new Location(3, 3)));
	rover = new Rover(this, new Location(1, 1), Location.North, Location.North);
	addPeon(rover);
    }

    public void dispose(){
	frame.dispose();
    }

    public boolean isVisible(){
	return frame.isVisible();
    }


    public void display(){
	mesh  = new Mesh();
	drawing.add(mesh);
	ListIterator iter = peonsIterator();
	while(iter.hasNext())drawing.add((EzdGlyph)iter.next());
	view.add(this.drawing);
	//	view.setBackground(Color.red);
	view.setBackground(Color.white);
	frame.setTitle(label);
	frame.getContentPane().setLayout(new BorderLayout());
	frame.getContentPane().add("Center", scrollPane );
	view.addKeyListener(new KeyAdapter(){
		public void keyPressed(KeyEvent ke){
		    if(ke.getKeyCode() == KeyEvent.VK_Q)frame.dispose();
		}
	    });
	frame.setVisible(true);
    }

    
    public static void main(String[] args){
	Grid grid = new Grid(6, 12, 100, 100);
	grid.display();
    }

    /* non-static innner class for convenience */
    class Mesh extends EzdGlyph {
	EzdGraphic[] grid;

	Mesh(){
	    grid = new EzdGraphic[ rows + columns + 2];
	    for(int i = 0; i <=  rows; i++){
		grid[i] = 
		    new EzdLine(0, i * height, totalWidth, i * height, Color.black);
	    }
	    for(int i = 0; i <= columns; i++){
		grid[i + rows + 1] =
		    new EzdLine(i * width, 0, i * width, totalHeight, Color.black);
	    }
	    draw();
	}

	void draw() {  drawAs(grid);	}
    }


}

