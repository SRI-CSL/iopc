package GraphAct;

import java.awt.Color;

public class Edge {
    int from, to;
    Color color;
    String label;
    public String toString(){
	return 
	    "(edge " + 
	    label +
	    " " +
	    from + 
	    " " + 
	    to + 
	    " " + 
	    Colour.color2String(color) + ")";
    }

    public boolean equals(Object obj){
	if(!(obj instanceof Edge)) return false;
	return 
	    (this.from == ((Edge)obj).from) &&
	    (this.to == ((Edge)obj).to) &&
	    this.label.equals(((Edge)obj).label);
    }
}
