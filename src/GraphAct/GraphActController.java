package GraphAct;

import java.util.ArrayList;
import java.util.ListIterator;


public class GraphActController {
    private final ArrayList peons = new ArrayList();  

    public void addPeon(Object peon){
	peons.add(peon);
    }

    public ListIterator peonsIterator(){
	return peons.listIterator();
    }

    public GraphActThing get(String label){
	GraphActThing g;
	ListIterator iter = peons.listIterator();
	while(iter.hasNext()){
	    g = (GraphActThing)iter.next();
	    if(g.getGraphActLabel().equals(label)){
		return g;
	    }
	}
	return null;
    }

    public void evaluate(Command cmd){
	String lab = cmd.getGraphActLabel();
	String[][] cmds = cmd.getCommands();
	for(int i = 0; i < cmds.length; i++){
	    pause(500);
	    String olab = cmds[i][0];
	    GraphActThing ob = get(olab);
	    if(ob != null)((GraphActPeon)ob).evaluate(cmds[i]);
	}
    }

    public static void pause(int millsecs){
	try {
	    Thread.currentThread().sleep(millsecs);
	}catch(Exception e){};
    }
    
}
