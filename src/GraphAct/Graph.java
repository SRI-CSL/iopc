package GraphAct;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.BorderLayout;
import java.awt.event.KeyEvent;
import java.awt.event.KeyAdapter;
import java.awt.event.MouseWheelListener;
import java.awt.event.MouseWheelEvent;

import javax.swing.JScrollPane;
import javax.swing.JSlider;
import javax.swing.BoundedRangeModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import java.util.ArrayList;
import java.util.ListIterator;

import Ezd.EzdView;
import Ezd.EzdDrawing;

public class Graph extends Skeleton implements GraphActWindowThing {
    private final EzdView  view = new EzdView();
    private final EzdDrawing  drawing = new EzdDrawing();
    private final ArrayList things = new ArrayList();
    private final ArrayList lines = new ArrayList();
    private final CJFrame frame = new CJFrame();
    private final JScrollPane scrollPane  = new JScrollPane(view);
    private final JSlider zoomer =  new JSlider(JSlider.HORIZONTAL);
    private final BoundedRangeModel model = zoomer.getModel();

    Graph(Skeleton sk){	super(sk);   }
     
    public void extend(Extension ex){
	Skeleton sk = ex.getSkeleton();
	ArrayList newNodes = sk.getNodes();
	ArrayList newEdges = sk.getEdges();
	addNodes(newNodes);
	addEdges(newEdges);
	this.resetRanks();
	this.computeRanks();
	ArrayList newThings = new ArrayList();
	ArrayList newLines = new ArrayList();
	processNodes(newNodes, newThings);
	addThings(newThings);
	processEdges(newEdges, newLines);
	addLines(newLines);
	drawing.removeAll();
	reRankThings();
	addLines2Display(lines);
	addThings2Display(things);
	refresh();
    }

    public void delete(Deletion del){
	Skeleton sk = del.getSkeleton();
	ArrayList delNodes = sk.getNodes();
	ArrayList delEdges = sk.getEdges();
	removeNodes(delNodes);
	removeEdges(delEdges);
	removeLines(delEdges);
	removeThings(delNodes);
	refresh();
    }

    public void processNodes(ArrayList nodes, ArrayList things){
	ListIterator iter = nodes.listIterator();
	while(iter.hasNext()){
	    Node node = (Node)iter.next();
	    if(node.type == Node.MOLECULE){
		things.add(new Circle(node, frame));
	    } else if(node.type == Node.RULE){
		things.add(new Rectangle(node, frame));
	    } else {
		IO.err.println("Unknown shape: " + node.shape);
		return;
	    }
	}
    }

    public void processEdges(ArrayList edges, ArrayList lines){
	ListIterator iter = edges.listIterator();
	while(iter.hasNext()){
	    Edge edge = (Edge)iter.next();
	    Thing thingA = (Thing)things.get(Graph.this.getIndexFromId(edge.from));
	    Thing thingB = (Thing)things.get(Graph.this.getIndexFromId(edge.to));
	    Line line = new Line(edge, thingA, thingB, frame);
	    lines.add(line);
	}
    }


    public void addLines(ArrayList lines){
	ListIterator iter = lines.listIterator();
	while(iter.hasNext())this.lines.add(iter.next());
    }

    public void removeLine(Edge edge){
	Line line = null;
	ListIterator iter = lines.listIterator();
	while(iter.hasNext()){
	    line = (Line)iter.next();
	    if(line.edge.equals(edge)){ 
		this.lines.remove(line);
		break;
	    }
	    line = null;
	}
	if(line != null)drawing.remove(line);
    }

    public void removeLines(ArrayList edges){
	ListIterator iter = edges.listIterator();
	while(iter.hasNext()){
	    Edge edge = (Edge)iter.next();
	    removeLine(edge);
	}
    }
    

    public void addThings(ArrayList things){
	ListIterator iter = things.listIterator();
	while(iter.hasNext())this.things.add(iter.next());
    }

    public void removeThing(Node node){
	Thing thing = null;
	ListIterator iter = things.listIterator();
	while(iter.hasNext()){
	    thing = (Thing)iter.next();
	    if(thing.node.equals(node)){ 
		this.things.remove(thing);
		break;
	    }
	    thing = null;
	}
	if(thing != null)drawing.remove(thing);
    }

    public void removeThings(ArrayList nodes){
	ListIterator iter = nodes.listIterator();
	while(iter.hasNext()){
	    Node node = (Node)iter.next();
	    removeThing(node);
	}
    }

   public void addLines2Display(ArrayList lines){
	ListIterator iter = lines.listIterator();
	while(iter.hasNext())drawing.add((Line)iter.next());
    }

    public void reRankThings(){
	ListIterator iter = things.listIterator();
	while(iter.hasNext())((Thing)iter.next()).reRank();
    }

    public void addThings2Display(ArrayList things){
	boolean changed = false;
	Dimension d = frame.getSize();
	ListIterator iter = things.listIterator();
	while(iter.hasNext()){
	    Thing thing = (Thing)iter.next();
	    drawing.add(thing);
	    if(thing.x + 100 >= d.width){
		d.width = thing.x + 100;
		changed = true;
	    }
	    if(thing.y + 100 >= d.height){
		d.height = thing.y + 100;
		changed = true;
	    }
	}
	if(changed){
	    view.setOrigonalSize(d);
	    view.setSize(d);
	}
    }

    public void refresh(){
	ListIterator iter = lines.listIterator();
	while(iter.hasNext())((Line)iter.next()).draw();
	iter = things.listIterator();
	while(iter.hasNext())((Thing)iter.next()).draw(false);
    }
    
    public void zoom(int factor){
	double x = Math.pow(10, (factor - 50.0)/50.0);
	view.zoom(x);
	view.repaint();
    }

    public void dispose(){
	frame.dispose();
    }

    public boolean isVisible(){
	return frame.isVisible();
    }

    public void display(){
	new Thread(){
	    public void run(){
		frame.setTitle(Graph.this.getGraphActLabel());
		frame.getContentPane().setLayout(new BorderLayout());
		frame.getContentPane().add(Graph.this.scrollPane, BorderLayout.CENTER);
		frame.getContentPane().add(zoomer, BorderLayout.NORTH);
		if(Graph.this.iopMenuBar != null){
		    frame.setJMenuBar(Graph.this.iopMenuBar);
		}
		Graph.this.view.setSize(frame.getSize());
		Graph.this.view.add(Graph.this.drawing);
		Graph.this.view.setBackground(Color.lightGray);
		Graph.this.computeRanks();
		processNodes(Graph.this.getNodes(), Graph.this.things);
		processEdges(Graph.this.getEdges(), Graph.this.lines);
		addLines2Display(Graph.this.lines);
		addThings2Display(Graph.this.things);
		frame.validate();
		frame.setVisible(true);
		Graph.this.view.setOrigonalSize(Graph.this.view.getSize());
		Graph.this.view.addKeyListener(new KeyAdapter(){
			public void keyPressed(KeyEvent ke){
			    if(ke.getKeyCode() == KeyEvent.VK_Q)frame.dispose();
			}
		    });
		Graph.this.view.addMouseWheelListener(new MouseWheelListener(){
			public void mouseWheelMoved(MouseWheelEvent mwe){
			    model.setValue(model.getValue() + mwe.getWheelRotation());
			    zoom(model.getValue());
			}
		    });
		zoomer.addChangeListener(new ChangeListener(){
			public void stateChanged(ChangeEvent e) {
			    JSlider source = (JSlider)e.getSource();
			    zoom(source.getValue());
			}
		    });
		view.repaint();

	    }
	}.start();
    }
    
}





