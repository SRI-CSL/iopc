package GraphAct;

import java.util.ArrayList;
import java.util.ListIterator;

public class Main {

    public static final boolean DEBUG = false;
    private static final ArrayList graphActThings = new ArrayList();

    protected static GraphActWindowThing getGraphActWindowThing(String label){
	ListIterator iter = graphActThings.listIterator();
	while(iter.hasNext()){
	    GraphActWindowThing g = (GraphActWindowThing)iter.next();
	    if(g.getGraphActLabel().equals(label)) return g;
	}
	return null;
    }


    public static void graphActCommandLoop(){
	while(true){
	    ActorMsg amsg = ActorMsg.readActorMsg();
	    if(DEBUG)IO.err.println(amsg);
	    dispatch(amsg);
	}
    }
    
    public static void main(String[] args){
	// a no-op in this case but here for a reminder
	IO.setIO(System.in, System.out, System.err);
	//	pfdTest();
	graphActCommandLoop();
    }

    public static void dispatch(ActorMsg amsg){
	switch(amsg.getType()){
	case ActorMsg.GRAPH: {
	    final Skeleton sk = new Skeleton(amsg.getBody());
	    final Graph gc = new Graph(sk);
	    if(gc != null){
		garbageCollect(gc.getGraphActLabel());
		graphActThings.add(gc);
		gc.display();
	    }
	    if(DEBUG)IO.err.println(gc);
	    break;
	}
	case ActorMsg.EXTEND: {
	    final Extension ex = new Extension(amsg.getBody());
	    if(DEBUG)IO.err.println(ex);
	    if(ex != null){
		if(DEBUG)IO.err.println(ex);
		final Object ob = getGraphActWindowThing(ex.getGraphActLabel());
		if(ob instanceof Graph){
		    final Graph gc = (Graph)ob;
		    if(gc != null) gc.extend(ex);
		}
	    }

	    break;
	}
	case ActorMsg.DELETE: {
	    final Deletion del = new Deletion(amsg.getBody());
	    if(del != null){
		if(DEBUG)IO.err.println(del);
		final Object ob = getGraphActWindowThing(del.getGraphActLabel());
		if(ob instanceof Graph){
		    final Graph gc = (Graph)ob;
		    if(gc != null) gc.delete(del);
		}
	    }
	    break;
	}
	case ActorMsg.STRING: {
	    final Rule rule = new Rule(amsg.getBody());
	    if(rule != null){
		rule.display();
	    }
	    break;		
	}
	case ActorMsg.GRID: {
	    final Grid grid = new Grid(amsg.getBody());
	    if(grid != null){
		garbageCollect(grid.getGraphActLabel());
		graphActThings.add(grid);
		grid.display();
	    }
	    break;
	}
	case ActorMsg.COMMAND: {
	    final Command command = new Command(amsg.getBody());
	    if(command != null){
		final Object ob = getGraphActWindowThing(command.getGraphActLabel());
		if(ob instanceof GraphActController){
		    final GraphActController mc = (GraphActController)ob;
		    if(mc != null) mc.evaluate(command);
		}	
	    }
	    break;	
	}
	case ActorMsg.CANVAS: {
	    final IOPCanvas canvas = new IOPCanvas(amsg.getBody());
	    if(canvas != null){
		garbageCollect(canvas.getGraphActLabel());
		graphActThings.add(canvas);
		canvas.display();
	    }
	    break;
	}
	case ActorMsg.PFDCANVAS: {
	    final PfdCanvas pfdcanvas = new PfdCanvas(amsg.getBody());
	    if(pfdcanvas != null){
		garbageCollect(pfdcanvas.getGraphActLabel());
		graphActThings.add(pfdcanvas);
		pfdcanvas.display();
	    }
	    break;
	}
	case ActorMsg.PFDEXTEND: {
	    final PfdExtension ex = new PfdExtension(amsg.getBody());
	    if(DEBUG)IO.err.println(ex);
	    if(ex != null){
		if(DEBUG)IO.err.println(ex);
		final Object ob = getGraphActWindowThing(ex.getGraphActLabel());
		if(ob instanceof PfdCanvas){
		    final PfdCanvas pfdc = (PfdCanvas)ob;
		    if(pfdc != null) pfdc.extend(ex);
		}
	    }

	    break;
	}
	case ActorMsg.PFDDELETE: {
	    final PfdDeletion del = new PfdDeletion(amsg.getBody());
	    if(del != null){
		if(DEBUG)IO.err.println(del);
		final Object ob = getGraphActWindowThing(del.getGraphActLabel());
		if(ob instanceof PfdCanvas){
		    final PfdCanvas pfdc = (PfdCanvas)ob;
		    if(pfdc != null) pfdc.delete(del);
		}
	    }
	    break;
	}
	default:
	    IO.err.println("Unknown Message = \"" + amsg.getBody() + "\"");
	}
    }

    
    private static void garbageCollect(String label){
	final Object ob = getGraphActWindowThing(label);
	if(ob != null){
	    GraphActWindowThing ogc = (GraphActWindowThing)ob;
	    if(!ogc.isVisible()){
		graphActThings.remove(ogc);
	    } else {
		graphActThings.remove(ogc);
		ogc.dispose();
	    }
	}
    }

    public static void pfdTest(){
	//PfdCanvas testing stuff
	PfdCanvas pfdCanvas = new PfdCanvas(PfdCanvas.testString);
	graphActThings.add(pfdCanvas);
	pfdCanvas.display();

	PfdExtension pfdExtension1 = new PfdExtension(PfdExtension.pfdextendString1);

	try {
	    Thread.currentThread().sleep(2000);
	}catch(Exception e){}

	pfdCanvas.extend(pfdExtension1);

	PfdDeletion pfdDeletion = new PfdDeletion(PfdDeletion.pfddeleteString);

	try {
	    Thread.currentThread().sleep(2000);
	}catch(Exception e){}

	pfdCanvas.delete(pfdDeletion);

	PfdExtension pfdExtension2 = new PfdExtension(PfdExtension.pfdextendString2);

	try {
	    Thread.currentThread().sleep(2000);
	}catch(Exception e){}

	pfdCanvas.extend(pfdExtension2);
    }

}



