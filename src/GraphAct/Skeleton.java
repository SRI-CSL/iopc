package GraphAct;

import java.util.ArrayList;
import java.util.ListIterator;
import java.util.Iterator;
import java.util.NoSuchElementException;

public class Skeleton implements GraphActThing {
    protected IOPMenuBar iopMenuBar;
    public static final boolean DEBUG = false;
    protected String label = "Anonymous";
    protected ArrayList nodes = new ArrayList();
    protected ArrayList edges = new ArrayList();
    private boolean preRanked = false;
    private int minRank = -1;
    private int maxRank = 0;

    public String toString(){
	ListIterator iter;
	String retval = "(graph\n";
	retval += " (label " + label + ")\n";
	retval += " (nodes ";
	iter = nodes.listIterator();
	while(iter.hasNext()) retval += "\n\t" + (Node)iter.next();
	retval += ")\n";
	retval += " (edges ";
	iter = edges.listIterator();
	while(iter.hasNext()) retval += "\n\t" + (Edge)iter.next();
	retval += ")\n";
	retval += ")";
	return retval;
    }

    Skeleton(){  }

    Skeleton(Skeleton sk){
	this.label = sk.getGraphActLabel();  //NB: structure sharing 
	this.nodes = sk.getNodes();       //NB: structure sharing
	this.edges = sk.getEdges();       //NB: structure sharing
	this.iopMenuBar =  sk.iopMenuBar; //NB: structure sharing
    }
    
    Skeleton(String string){
	Sexp list;
	try{
	    list = new Sexp(string);
	    Parser.parseSkeleton(this, list);
	}catch(ParseError pe){ IO.err.println(pe); }
    }

    public void add(Node node){	nodes.add(node);  }
    public void remove(Node node){ nodes.remove(node); }
    
    
    public void add(Edge edge){	edges.add(edge);  }
    public void remove(Edge edge){ edges.remove(edge); }

    
    
    public void addNodes(ArrayList nodes){
	ListIterator iter = nodes.listIterator();
	while(iter.hasNext()) this.nodes.add(iter.next());
    }

    public void removeNodes(ArrayList nodes){
	ListIterator iter = nodes.listIterator();
	while(iter.hasNext()) this.nodes.remove(iter.next());
    }

    public void addEdges(ArrayList edges){
	ListIterator iter = edges.listIterator();
	while(iter.hasNext()) this.edges.add(iter.next());
    }

    public void removeEdges(ArrayList edges){
	ListIterator iter = edges.listIterator();
	while(iter.hasNext()) this.edges.remove(iter.next());
    }

    public void resetRanks(){
	ListIterator iter = nodes.listIterator();
	while(iter.hasNext()){
	    Node n = (Node) iter.next();
	    n.xRank = 0;
	    n.yRank = 0;
	}
    }

    public void reportRanks(){
	ListIterator iter = nodes.listIterator();
	while(iter.hasNext()){
	    Node n = (Node) iter.next();
	    IO.err.println("Node " + n.label + " " + n.xRank + " " +  n.yRank);
	}
    }

    public void setGraphActLabel(String label){ this.label = label; }
    public String getGraphActLabel(){ return this.label; }

    public ArrayList getNodes(){ return this.nodes; }
    public ArrayList getEdges(){ return this.edges; }

    public int getIndexFromId(int id){
	int retval = -1;
	ListIterator iter = nodes.listIterator();
	while(iter.hasNext()){
	    retval++;
	    if(((Node)iter.next()).id == id) return retval;
	}
	return -1;
    }
    
    public Node getNode(int id){
	int index = getIndexFromId(id);
	if(index < 0) return null;
	return (Node)this.nodes.get(index);
    }

    public void computeChildren(){
	int size = this.edges.size();
	for(int j = 0; j < size; j++){
	    ListIterator iter = this.edges.listIterator();
	    while(iter.hasNext()){
		Edge e = (Edge)iter.next();
		Node from = getNode(e.from);
		Node to = getNode(e.to);
		if(!from.to.contains(to)) from.to.add(to);
		if(!to.from.contains(from)) to.from.add(from);
		if(!from.children.contains(to)) from.children.add(to);
		ArrayList kids = from.children;
		ListIterator kiter = kids.listIterator();
		while(kiter.hasNext()){
		    Node c = (Node)kiter.next();
		    if(!from.children.contains(c))from.children.add(c);
		}

	    }
	}	
    }
    
    public void computeRanks(){
	computeChildren();
	computeYRank();
	relaxYRank();
	computeXRank();
	tweak();
    }
    

    public void computeYRank(){

	//Maude already ranked them?
	ListIterator miter = nodes.listIterator();
	while(miter.hasNext()){
	    Node node = (Node)miter.next();
	    int level = node.level;
	    if(level >= 0){
		node.yRank = level;
		if(maxRank < level) maxRank = level;
		if(minRank < 0) 
		    minRank = level;
		else
		    minRank = (minRank < level) ? minRank : level;
		preRanked = true;
	    }
	}
	if(preRanked == true){
	    // IO.err.println("miniRank = " + minRank + " maxRank = " +  maxRank);
	    return;
	}

	//all rules get assigned 1
	ListIterator iter = nodes.listIterator();
	while(iter.hasNext()){
	    Node node = (Node)iter.next();
	    if(node.type == Node.RULE) node.yRank = 1;
	}

	int esize = this.edges.size();
	for(int k = 0; k < esize; k++){
	    boolean changed = false;
	    //push down molecules
	    iter = this.edges.listIterator();
	    while(iter.hasNext()){
		Edge e = (Edge)iter.next();
		Node from = getNode(e.from);
		if(from.type == Node.RULE){
		    Node to = getNode(e.to);
		    if(to.yRank <= from.yRank + 1){
			to.yRank = from.yRank + 1;
			changed = true;
		    }
		}
	    }
	    //push down rules
	    iter = this.edges.listIterator();
	    while(iter.hasNext()){
		Edge e = (Edge)iter.next();
		Node from = getNode(e.from);
		if(from.type == Node.MOLECULE){
		    Node to = getNode(e.to);
		    if(to.yRank <= from.yRank + 1){
			to.yRank = from.yRank + 1;
			changed = true;
		    }
		}
		
	    }
	    if(!changed) break;
	}

    }

    public void relaxYRank(){
	Iterator zeroNodes = nodesOfRank(0);
	while(zeroNodes.hasNext()){
	    Node znode = (Node)zeroNodes.next();
	    Iterator toNodes = znode.to.listIterator();
	    int minRank = 0;
	    while(toNodes.hasNext()){
		Node tonode = (Node)toNodes.next();
		if(minRank == 0) 
		    minRank = tonode.yRank;
		else
		    minRank = (tonode.yRank < minRank) ? tonode.yRank : minRank;
	    }
	    znode.yRank = (minRank - 1 < 0) ? 0 : minRank - 1;
	}
    }
    

    public void computeXRank(){

	if(preRanked == true){ 
	    preComputeXRank();
	    return;
	}

	//compute the top most nodes
	ArrayList topNodes = new ArrayList();
	ListIterator niter = nodes.listIterator();
	while(niter.hasNext()){
	    Node node = (Node)niter.next();
	    if(node.from.isEmpty()) topNodes.add(node);
	}

	//assign them ranks
	ListIterator tniter = topNodes.listIterator();
	int rank = 0;
	while(tniter.hasNext()){
	    Node tnode = (Node)tniter.next();
	    tnode.xRank = ++rank;
	}

	ArrayList previousLevel = topNodes;
	ArrayList nextLevel = new ArrayList();

	for(int k = 0; k < this.edges.size(); k++){
	    ListIterator iter = previousLevel.listIterator();
	    while(iter.hasNext()){
		Node pnode = (Node)iter.next();
		ListIterator piter = pnode.to.listIterator();
		while(piter.hasNext()){
		    Node ptonode = (Node)piter.next();
		    if(!nextLevel.contains(ptonode)) nextLevel.add(ptonode);
		    ptonode.avergageXRank();
		}

	    }
	    ArrayList temp = previousLevel;
	    previousLevel = nextLevel;
	    nextLevel = temp;
	    nextLevel.clear();
	}
    }

    
    Iterator nodesOfRank(final int n){
	return new Iterator(){
		ListIterator iter = nodes.listIterator();
		Object myNext = null;
		public boolean hasNext(){
		    if(myNext != null) return true;
		    while(iter.hasNext()){
			Node node = (Node)iter.next();
			if(node.yRank == n){
			    myNext = node;
			    return true;
			}
		    }
		    return false;
		}
		public Object next(){
		    if(myNext != null){
			Object retval = myNext;
			myNext = null;
			return retval;
		    } else {
			if(!hasNext())
			    throw new NoSuchElementException();
			else {
			    Object retval = myNext;
			    myNext = null;
			    return retval;
			}
		    }
		}
		public void remove(){
		    throw new UnsupportedOperationException();
		}
	    };
    }
    
    public void preComputeXRank(){
	Iterator tniter = nodesOfRank(minRank);
	int rank = 0;
	while(tniter.hasNext()){
	    Node tnode = (Node)tniter.next();
	    rank += 2;
	    tnode.xRank = rank;
	}
	for(int i = minRank + 1; i <= maxRank; i++){
	    Iterator iter = nodesOfRank(i);
	    while(iter.hasNext()){
		Node node = (Node)iter.next();
		node.avergageXRank();
	    }
	    tweak(i);
	}
    }

    public void tweak(){
	int index = 0;
	ListIterator iter = nodes.listIterator();
	while(iter.hasNext()){
	    index++;
	    Node f = (Node)iter.next();
	    ListIterator riter = nodes.listIterator(index);
	    while(riter.hasNext()){
		Node b = (Node)riter.next();
		if((f.xRank == b.xRank) && (f.yRank == b.yRank))
		    b.xRank += 2;
	    }
	}
    }

    public void tweak(int level){
	int index = 0;
	Iterator levelNodes = nodesOfRank(level);
	while(levelNodes.hasNext()){
	    index++;
	    Node f = (Node)levelNodes.next();
	    Iterator rNodes = nodesOfRank(level);
	    for(int i = 0; i < index; i++) rNodes.next();
	    while(rNodes.hasNext()){
		Node b = (Node)rNodes.next();
		if((f.xRank == b.xRank) && (f.yRank == b.yRank))
		    b.xRank += 2;
	    }
	}
    }
}







