package GraphAct;


import java.awt.event.KeyEvent;


public class Location {
	
    private int row, col;				

    public static final int North     = 0;
    public static final int East      = 1;
    public static final int South     = 2;
    public static final int West      = 3;

    public static final int Northwest = 4;
    public static final int Northeast = 5;
    public static final int Southwest = 6;
    public static final int Southeast = 7;
    
    
    public Location(int r, int c){
	row = r;
	col = c;
    }
    
    
    public int getRow(){ return row;  }
    
    public int getCol(){ return col; }
    
    public boolean equals(Object o){
	return( (o instanceof Location) 
		&& ((Location)o).row == this.row 
		&& ((Location)o).col == this.col );
    }
    
    public static int reverseDirection(int whichDir){
	switch (whichDir) {
	case North: return South;
	case South: return North;
	    
	case East:  return West;
	case West:  return East;
	    
	case Northeast:  return Southwest;
	case Northwest:  return Southeast;
	    
	case Southeast:  return Northwest;
	case Southwest:  return Northeast;
	    
	default:  throw new IndexOutOfBoundsException();
	}
    }
    
    public Location adjacentLocation(int whichDir){
	switch (whichDir) {
	case North: return new Location(row - 1, col);
	case South: return new Location(row + 1, col);
	    
	case East:  return new Location(row, col + 1);
	case West:  return new Location(row, col - 1);
	    
	case Northeast:  return new Location(row - 1, col + 1);
	case Northwest:  return new Location(row - 1, col - 1);
	    
	case Southeast:  return new Location(row + 1, col + 1);
	case Southwest:  return new Location(row + 1, col - 1);
	    
	default:  throw new IndexOutOfBoundsException();
	}
    }


    public String toString(){
	return "[" + row + ", " + col + "]";
    }
    

    public static int keyCodeToDirection(int num){
	switch (num) {
	case KeyEvent.VK_KP_UP:    case KeyEvent.VK_UP:
	    return Location.North;
	case KeyEvent.VK_KP_DOWN:  case KeyEvent.VK_DOWN: 
	    return Location.South;
	case KeyEvent.VK_KP_RIGHT: case KeyEvent.VK_RIGHT: 
	    return Location.East;
	case KeyEvent.VK_KP_LEFT:  case KeyEvent.VK_LEFT: 
	    return Location.West;
	default: throw new IndexOutOfBoundsException();
	}
    }
    
    public static int stringToDir(String s){
	char c = s.toLowerCase().charAt(0);
	switch(c){
	case 'n': return Location.North;
	case 'e': return Location.East;
	case 's': return Location.South;
	case 'w': return Location.West;
	default: return Location.North;
	}
    }
}



