package GraphAct;

import java.awt.Color;
import java.util.ArrayList;
import java.util.ListIterator;


public class Node {
    public static final int MOLECULE = 0;
    public static final int RULE = 1;
    public static final boolean DEBUG = false;
    int type;
    int id;
    int level = -1;
    int xRank = 0;
    int yRank = 0;
    String shape = "circle";
    String label = "unnamed";
    String onclick = "";
    Color color = Color.red;
    Color border = Color.green;
    ArrayList from = new ArrayList();
    ArrayList to = new ArrayList();
    ArrayList children = new ArrayList();
    public String toString(){
	return 
	    "(node \"" + 
	    label + 
	    "\" " +
	    id + 
	    " [" + 
	    xRank + 
	    "," + 
	    yRank + 
	    "]" + 
	    //	    children.size() + 
	    //	    " " + 
	    //	    shape + 
	    //	    " " + 
	    //	    " " + 
	    //	    Colour.color2String(color) + 
	    //	    " " + 
	    //	    Colour.color2String(border) + 
	    //	    " " +
	    //	    onclick + 
	    ")";
    }

    public void avergageXRank(){
	int total = 0;
	int rsize = 0;
	int size = this.from.size();
	if(size != 0){
	    ListIterator iter = this.from.listIterator();
	    while(iter.hasNext()){ 
		Node from = (Node)iter.next();
		if(from.level < this.level){
		    if(DEBUG)IO.err.println("\t" + from);
		    total += from.xRank;
		    rsize++;
		}
	    }
	    if(rsize > 0) 
		this.xRank = (total / rsize);
	    else 
		this.xRank = 1;
	    if(DEBUG){
		IO.err.println(this);
		IO.err.println(" total = " + total + 
				   " size = " + size + 
				   " rsize = " + rsize + 
				   " rank = " + this.xRank);
	    }
	} else {
	    this.xRank = 1;
	}
    }

    public boolean equals(Object obj){
	if(!(obj instanceof Node)) return false;
	return this.id == ((Node)obj).id;
    }

}
