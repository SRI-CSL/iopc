package GraphAct;

import java.awt.Color;

import Ezd.EzdGlyph;
import Ezd.EzdFillRectangle;

public class Obstacle extends EzdGlyph implements GraphActThing {
    private Grid grid;
    private Location loc;
    private String label;


    Obstacle(Grid grid, Location loc){
	this.grid = grid;
	this.loc = loc;
	draw();
    }

    /* (obstacle id r c ) */
    Obstacle(Grid grid, Sexp list) throws ParseError {
	int row, column;
	this.grid = grid;
	if(list.length() !=  4)
	    throw new ParseError("Obstacle: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Obstacle: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("Obstacle"))
	    throw new ParseError("Obstacle: didn't start with \"Obstacle\"");
	if(!(list.get(1) instanceof String))
	    throw new ParseError("Obstacle: label not a String");
	label = (String)list.get(1);
	if(!(list.get(2) instanceof String))
	    throw new ParseError("Obstacle: row not a String");
	row = new Integer((String)list.get(2)).intValue();
	if(!(list.get(3) instanceof String))
	    throw new ParseError("Obstacle: column not a String");
	column = new Integer((String)list.get(3)).intValue();
	loc = new Location(row, column);
	draw();
    }

    public String getGraphActLabel(){ return label; }
    public void setGraphActLabel(String label){ this.label = label; }


    void draw(){
	int width = grid.getWidth(), height = grid.getHeight();
	int xPos = loc.getCol() * width, yPos = loc.getRow() * height;
	drawAs(new EzdFillRectangle(xPos, yPos, width, height, Color.black));
    }
}
