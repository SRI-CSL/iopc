package GraphAct;

import java.awt.Polygon;
import java.awt.Color;
import java.awt.Toolkit;
import java.awt.event.MouseEvent;

import java.util.ArrayList;

import Ezd.EzdGlyph;
import Ezd.EzdFillPolygon;
import Ezd.EzdPolygon;
import Ezd.EzdView;

public class Rover extends EzdGlyph  implements GraphActPeon {

    private Location loc;
    private Grid grid;
    private String label;

    private int roverDir;
    private boolean is45OffPerp = false;
    private int armDir;

    private int batteryLevel = 100;
    private int batteryWidth;
    private int batteryHeight;

    private int width;
    private int height;
    private int armLength;
    private int armWidth;

    int[][] bxPoints; 
    int[][] byPoints; 
    int[][] rxPoints; 
    int[][] ryPoints; 
    int[][] axPoints; 
    int[][] ayPoints; 

    public String getGraphActLabel(){ return label; }
    public void setGraphActLabel(String label){ this.label = label; }

    public void drainBattery(int percent){
	if(percent >= batteryLevel)
	    batteryLevel = 0;
	else
	    batteryLevel -= percent;
    }

    public void evaluate(String[][] cmds){
	for(int i = 0; i < cmds.length; i++)
	    this.evaluate(cmds[i]);
    }

    /*  (rover id rR cR rDir aDir)  */
    Rover(Grid grid, Sexp list)  throws ParseError {
	int row, column;
	this.grid = grid;
	if(list.length() !=  6)
	    throw new ParseError("Rover: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Rover: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("Rover"))
	    throw new ParseError("Rover: didn't start with \"Rover\"");
	if(!(list.get(1) instanceof String))
	    throw new ParseError("Rover: label not a String");
	this.label = (String)list.get(1);
	if(!(list.get(2) instanceof String))
	    throw new ParseError("Rover: row not a String");
	row = new Integer((String)list.get(2)).intValue();
	if(!(list.get(3) instanceof String))
	    throw new ParseError("Rover: column not a String");
	column = new Integer((String)list.get(3)).intValue();
	this.loc = new Location(row, column);
	if(!(list.get(4) instanceof String))
	    throw new ParseError("Rover: roverDir not a String");
	this.roverDir = Location.stringToDir((String)list.get(4));
	if(!(list.get(5) instanceof String))
	    throw new ParseError("Rover: armDir not a String");
	this.armDir = Location.stringToDir((String)list.get(5));
    	this.width = grid.getWidth();
	this.height = grid.getHeight();
	armLength = width;
	armWidth = width/5;
	batteryHeight = width/4;
	makeArrays();
	draw();
    }

    Rover(Grid grid, Location loc, int roverDir, int armDir){
	this.grid = grid;
	this.loc = loc;
	this.width = grid.getWidth();
	this.height = grid.getHeight();
	this.roverDir = roverDir;
	this.armDir = armDir;
	armLength = width;
	armWidth = width/5;
	makeArrays();
	draw();
    }


    void makeArrays(){
	int[][] rxpnts 
	    = {  
		{width/2, width, 0},      //north
		{0, width, 0},            //east
		{0, width/2, width},      //south
		{0, width, width}         //west
	    };
	rxPoints = rxpnts;
	int[][] rypnts 
	    = {
		{0, height, height},
		{0, height/2, height},
		{0, height, 0},
		{height/2, 0, height}
	    };
	ryPoints = rypnts;
	int[][] axpnts 
	    = {
		{ -armWidth/2, -armWidth/2, armWidth/2, armWidth/2},  //north
		{ 0, armLength, armLength, 0},                        //east
		{ -armWidth/2, -armWidth/2, armWidth/2, armWidth/2},  //south
		{ 0, -armLength, -armLength, 0}                       //west
	    };
	axPoints = axpnts;
	int[][] aypnts 
	    = {
		{ 0, -armLength,  -armLength, 0},                        //north
		{ -armWidth/2, -armWidth/2, armWidth/2, armWidth/2},     //east
		{ 0, armLength, armLength, 0},                           //south
		{ -armWidth/2, -armWidth/2, armWidth/2, armWidth/2}      //west
	    };
	int bh = batteryHeight;
	int bh2 = batteryHeight/2;
	int[][] bxpnts
	    =
	    {
		{bh2, bh2, width - bh2, width - bh2},  //north
		{bh, 0, 0, bh},                        //east
		{width - bh2, width - bh2, bh2, bh2},  //south
		{width - bh, width, width, width - bh} //west
	    };
	bxPoints = bxpnts; 
	int[][] bypnts
	    =
	    {
		{height - bh, height, height, height - bh}, //north
		{bh2, bh2, height - bh2, height - bh2},     //east
		{bh, 0, 0, bh},                             //south
		{width - bh2, width - bh2,   bh2,   bh2}    //west
	    };
	byPoints = bypnts;
	ayPoints = aypnts;
    }
    
    Polygon makeFuel(){
	//	System.err.println(batteryLevel);
	int bh = batteryHeight;
	int bh2 = batteryHeight/2;
	int fl = bh2 + ((width - bh) * batteryLevel) / 100;
	int[][] bxpnts
	    =
	    {
		{bh2, bh2, fl, fl},                                  //north
		{bh, 0, 0, bh},                                      //east
		{width - bh2, width - bh2, width - fl, width - fl},  //south
		{width - bh, width, width, width - bh}               //west
	    };
	int[][] bypnts
	    =
	    {
		{height - bh, height, height, height - bh},         //north
		{bh2, bh2, fl, fl},                                 //east
		{bh, 0, 0, bh},                                     //south
		{width - bh2, width - bh2, width - fl, width - fl}  //west
	    };
	return new Polygon(bxpnts[roverDir], bypnts[roverDir], 4);
    }

    public void mousePressed(EzdView v, MouseEvent me, int x, int y ) {
	//new commands
	if(me.isControlDown() && me.isAltDown()){
	    drive();
	} else if(me.isShiftDown() && me.isAltDown()){
	    rotate();
	} 
	//old commands
	else if(me.isControlDown()){
	    move();
	} else if(me.isShiftDown()){
	    rotate();
	} else {
	    swing();
	}
    }

    void move(){
	if(batteryLevel > 0){
	    loc = loc.adjacentLocation(roverDir);
	    drainBattery(5);
	    draw();
	} else {
	    Toolkit.getDefaultToolkit().beep();
	}
    }

    void drive(){
	if(is45OffPerp) return;
	loc = loc.adjacentLocation(roverDir);
	draw();
    }
    
    void rotate(){ 
	if(is45OffPerp){
	    roverDir = (roverDir + 1) % 4; 
	    is45OffPerp = false;
	} else {
	    is45OffPerp = true;
	}
	draw();	
    }

    void drain(String num){ 
	int n = Integer.parseInt(num);
	drainBattery(n);
	draw();	
    }
    void rotate(String dir){ 
	if(batteryLevel > 0){
	    roverDir = Location.stringToDir(dir);  
	    drainBattery(5);
	    draw();	
	} else {
	    Toolkit.getDefaultToolkit().beep();
	}
    }
    
    void swing(){ 
	armDir = (armDir + 1) % 4; 
	draw();	
    }

    void recharge(){ 
	batteryLevel = 100; 
	draw();	
    }
    
    void swing(String dir){ 
	if(batteryLevel > 0){
	    armDir = Location.stringToDir(dir);    
	    drainBattery(5);
	    draw();	
	} else {
	    Toolkit.getDefaultToolkit().beep();
	}
    }

    void draw(){
	int xPos, yPos;
	Polygon rover, arm, battery, fuel;
	xPos = loc.getCol() * grid.getWidth();
	yPos = loc.getRow() * grid.getHeight();
	rover = new Polygon(rxPoints[roverDir], ryPoints[roverDir], 3);
	rover.translate(xPos, yPos);
	arm =  new Polygon(axPoints[(roverDir + armDir)%4], ayPoints[(roverDir + armDir)%4], 4);
	arm.translate(xPos + width/2, yPos + height/2);
	battery = new Polygon(bxPoints[roverDir], byPoints[roverDir], 4);
	battery.translate(xPos, yPos);
	fuel = makeFuel();
	fuel.translate(xPos, yPos);
	//	System.err.println("drawing battery");
	drawAs(new EzdFillPolygon(rover, is45OffPerp ? Color.red : Colour.purple ),
	       new EzdPolygon(rover, Color.black),
	       new EzdFillPolygon(battery, Color.red),
	       new EzdFillPolygon(fuel, Color.green),
	       new EzdPolygon(battery, Color.black),
	       new EzdFillPolygon(arm, Colour.lightBlue),
	       new EzdPolygon(arm, Color.black));
    }

    public void evaluate(String[] cmd){
	//new commands
	if((cmd.length == 2) && cmd[1].toLowerCase().equals("drive")){
	    this.drive();
	} else if ((cmd.length == 2) && cmd[1].toLowerCase().equals("rotate")){
	    this.rotate();
	} else if ((cmd.length == 2) && cmd[1].toLowerCase().equals("swing")){
	    this.swing();
	} else if ((cmd.length == 2) && cmd[1].toLowerCase().equals("recharge")){
	    this.recharge();
	} else if ((cmd.length == 3) && cmd[1].toLowerCase().equals("drain")){
	    this.drain(cmd[2]);
	} 
	//old commands
	else if((cmd.length == 2) && cmd[1].toLowerCase().equals("move")){
	    this.move();
	} else if ((cmd.length == 3) && cmd[1].toLowerCase().equals("rotate")){
	    this.rotate(cmd[2]);
	} else if ((cmd.length == 3) && cmd[1].toLowerCase().equals("swing")){
	    this.swing(cmd[2]);
	} else {
	    IO.err.println("Rover.evaluate didn't understand cmd");
	}
	
    }

    /*
      New commands
      drive
      swing    % swing the arm clockwise
      rotate   % 2 rotates rotates the rover 90, can't drive while only 45 off a perpidicular
      recharge % recharges battery

    */
    
}
