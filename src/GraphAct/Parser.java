package GraphAct;

import java.util.StringTokenizer;
import java.util.ListIterator;

public class Parser {
    public static final String whitespace = " \t\n\r";
    public static final String delims = "()\"";

    public static boolean isWhitespace(String tok){
	return whitespace.indexOf(tok) >= 0;
    }
    public static boolean isParen(String tok){
	return delims.indexOf(tok) >= 0;
    }
    public static String nextToken(StringTokenizer s){
	String retval = "";
	String tok = s.nextToken();
	while(isWhitespace(tok)) tok = s.nextToken();
	if(tok.equals("\"")){
	    while(true){
		tok = s.nextToken();
		if(tok.equals("\"")) break;
		retval += tok;
	    }
	} else retval = tok;
	return retval;
    }

    public static void parseGrid(Grid g, Sexp list)
	throws ParseError {
	if((list.length() < 4) || (list.length() > 5))
	    throw new ParseError("Grid: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Grid: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("Grid"))
	    throw new ParseError("Grid: didn't start with \"Grid\"");
	if(!(list.get(1) instanceof Sexp))
	    throw new ParseError("Grid: label attribute not a Sexp");
	parseLabel(g, (Sexp)list.get(1));
	if(!(list.get(2) instanceof Sexp))
	    throw new ParseError("Grid: rows attribute not a Sexp");
	parseRows(g, (Sexp)list.get(2));
	if(!(list.get(3) instanceof Sexp))
	    throw new ParseError("Grid: columns attribute not a Sexp");
	parseColumns(g, (Sexp)list.get(3));
	if(list.length() == 5){
	    if(!(list.get(3) instanceof Sexp))
		throw new ParseError("Grid: contents attribute not a Sexp");
	    parseContents(g, (Sexp)list.get(4));
	}
	g.setTotalWidth();
	g.setTotalHeight();
	g.setViewSize();
	g.setFrameSize();
    }
    
    public static void parseRows(Grid g, Sexp list) throws ParseError {
	if(list.length() != 2)
	    throw new ParseError("Grid: rows attribute not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Grid: rows attribute didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("rows"))
	    throw new ParseError("Grid: rows attribute didn't start with \"rows\"");
	if(!(list.get(1) instanceof String))
	    throw new ParseError("Grid: rows not a String");
	g.setRows(Integer.parseInt((String)list.get(1)));
    }

    public static void  parseColumns(Grid g, Sexp list) throws ParseError {
	if(list.length() != 2)
	    throw new ParseError("Grid: columns attribute not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Grid: columns attribute didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("columns"))
	    throw new ParseError("Grid: columns attribute didn't start with \"columns\"");
	if(!(list.get(1) instanceof String))
	    throw new ParseError("Grid: columns not a String");
	g.setColumns(Integer.parseInt((String)list.get(1)));
    }

    public static void parseContents(Grid g, Sexp list) throws ParseError {
	ListIterator iter = list.listIterator();
	if(!iter.hasNext())
	    throw new ParseError("Grid: contents attribute empty");
	Object first = iter.next();
	if(!(first instanceof String))
	    throw new ParseError("Grid: contents attribute didn't start with a String");
	if(!((String)first).equalsIgnoreCase("contents"))
	    throw new ParseError("Grid: contents attribute didn't start with \"contents\"");
	while(iter.hasNext()){
	    Object next = iter.next();
	    if(!(next instanceof Sexp))
		throw new ParseError("Grid: contents element not an Sexp");
	    parseObject(g, (Sexp)next);
	}
    }

    public static void parseObject(Grid g, Sexp list) throws ParseError {
	if(list.length() < 4)
	    throw new ParseError("Grid: contents element too small");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Grid: contents element didn't start with a String");
	if(((String)list.get(0)).equalsIgnoreCase("rover")){
	    g.add(new Rover(g, list));
	} else if(((String)list.get(0)).equalsIgnoreCase("obstacle")){
	    g.add(new Obstacle(g, list));
	} else {
	    throw new ParseError("Grid: contents element not recognized");
	}
    }

    public static void parseSkeleton(Skeleton g, Sexp list)
	throws ParseError {
	if((list.length() !=  4) &&  (list.length() !=  5))
	    throw new ParseError("Skeleton: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Skeleton: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("Graph"))
	    throw new ParseError("Skeleton: didn't start with \"Graph\"");
	if(!(list.get(1) instanceof Sexp))
	    throw new ParseError("Skeleton: label attribute not a Sexp");
	parseLabel(g, (Sexp)list.get(1));
	if(!(list.get(2) instanceof Sexp))
	    throw new ParseError("Skeleton: node list not a Sexp");
	parseNodeList(g, (Sexp)list.get(2));
	if(!(list.get(3) instanceof Sexp))
	    throw new ParseError("Skeleton: edge list not a Sexp");
	parseEdgeList(g, (Sexp)list.get(3));
	if(list.length() ==  5){
	    if(!(list.get(4) instanceof Sexp))
		throw new ParseError("Skeleton: MenuBar not a Sexp");
	    g.iopMenuBar = new IOPMenuBar((Sexp)list.get(4));
	}
    }
    
    public static void parseExtension(Extension g, Sexp list)
	throws ParseError {
	if(list.length() !=  5)
	    throw new ParseError("Extension: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Extension: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("Extend"))
	    throw new ParseError("Extension: didn't start with \"Extend\"");
	if(!(list.get(1) instanceof Sexp))
	    throw new ParseError("Extension: focus attribute not a Sexp");
	parseFocus(g, (Sexp)list.get(1));
	if(!(list.get(2) instanceof Sexp))
	    throw new ParseError("Extension: label attribute not a Sexp");
	parseLabel(g, (Sexp)list.get(2));
	if(!(list.get(3) instanceof Sexp))
	    throw new ParseError("Extension: node list not a Sexp");
	parseNodeList(g.getSkeleton(), (Sexp)list.get(3));
	if(!(list.get(4) instanceof Sexp))
	    throw new ParseError("Extension: edge list not a Sexp");
	parseEdgeList(g.getSkeleton(), (Sexp)list.get(4));
    }


    public static void parseDeletion(Deletion g, Sexp list)
	throws ParseError {
	if(list.length() !=  4)
	    throw new ParseError("Deletion: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Deletion: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("Delete"))
	    throw new ParseError("Deletion: didn't start with \"Delete\"");
	if(!(list.get(1) instanceof Sexp))
	    throw new ParseError("Deletion: label attribute not a Sexp");
	parseLabel(g, (Sexp)list.get(1));
	if(!(list.get(2) instanceof Sexp))
	    throw new ParseError("Deletion: node list not a Sexp");
	parseNodeList(g.getSkeleton(), (Sexp)list.get(2));
	if(!(list.get(3) instanceof Sexp))
	    throw new ParseError("Deletion: edge list not a Sexp");
	parseEdgeList(g.getSkeleton(), (Sexp)list.get(3));
    }

    public static void parseFocus(Extension g, Sexp list)
	throws ParseError {
	if(list.length() != 2)
	    throw new ParseError("Extension: focus attribute not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Extension: focus attribute didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("focus"))
	    throw new ParseError("Extension: label attribute didn't start with \"focus\"");
	if(!(list.get(1) instanceof String))
	    throw new ParseError("Extension: focus not a String");
	g.setFocus((String)list.get(1));
    }

    public static void parseLabel(GraphActThing g, Sexp list)
	throws ParseError {
	if(list.length() != 2)
	    throw new ParseError("GraphActThing: label attribute not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("GraphActThing: label attribute didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("label"))
	    throw new ParseError("GraphActThing: label attribute didn't start with \"label\"");
	if(!(list.get(1) instanceof String))
	    throw new ParseError("GraphActThing: label not a String");
	g.setGraphActLabel((String)list.get(1));
    }

    public static void parseNodeList(Skeleton g, Sexp list)
	throws ParseError {
	ListIterator iter = list.listIterator();
	if(!iter.hasNext())
	    throw new ParseError("Skeleton: node list empty");
	Object first = iter.next();
	if(!(first instanceof String)) 
	    throw new ParseError("Skeleton: node list didn't start with a String");
	if(!((String)first).equalsIgnoreCase("nodes"))
	    throw new ParseError("Skeleton: node list didn't start with \"Nodes\": " +
				 (String)first);
	while(iter.hasNext()){
	    Object next = iter.next();
	    if(!(next instanceof Sexp))
		throw new ParseError("Skeleton: Node not an Sexp");
	    parseNode(g, (Sexp)next);
	}
    }

    public static void parseEdgeList(Skeleton g, Sexp list)
	throws ParseError {
	ListIterator iter = list.listIterator();
	if(!iter.hasNext())
	    throw new ParseError("Skeleton: edge list empty");
	Object first = iter.next();
	if(!(first instanceof String)) 
	    throw new ParseError("Skeleton: edge list didn't start with a String");
	if(!((String)first).equalsIgnoreCase("edges"))
	    throw new ParseError("Skeleton: edge list didn't start with \"Edges\": " + 
				 (String)first);
	while(iter.hasNext()){
	    Object next = iter.next();
	    if(!(next instanceof Sexp))
		throw new ParseError("Skeleton: Edge not an Sexp");
	    parseEdge(g, (Sexp)next);
	}
    }

    public static void parseNode(Skeleton g, Sexp list)
	throws ParseError {
	Node node = new Node();
	if(list.length() != 3)
	    throw new ParseError("Skeleton: Node not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Skeleton: Node didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("Node"))
	    throw new ParseError("Skeleton: Node didn't start with \"Node\"");
	if(!(list.get(1) instanceof String))
	    throw new ParseError("Skeleton: Node id not a String");
	node.id = Integer.parseInt((String)list.get(1));
	if(!(list.get(2) instanceof Sexp))
	    throw new ParseError("Skeleton: Node attribute list not a Sexp");
	ListIterator iter = ((Sexp)list.get(2)).listIterator();
	while(iter.hasNext()){
	    Object next = iter.next();
	    if(!(next instanceof Sexp))
		throw new ParseError("Skeleton: Node attribute not an Sexp");
	    parseNodeAttribute(node, (Sexp)next);
	}
	g.add(node);
    }

    public static void parseEdge(Skeleton g, Sexp list)
	throws ParseError {
	Edge edge = new Edge();
	if(list.length() != 4)
	    throw new ParseError("Skeleton: Edge not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Skeleton: Edge didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("Edge"))
	    throw new ParseError("Skeleton: Edge didn't start with \"Edge\"");
	if(!(list.get(1) instanceof String))
	    throw new ParseError("Skeleton: Edge from node not a String");
	edge.from = Integer.parseInt((String)list.get(1));
	if(!(list.get(2) instanceof String))
	    throw new ParseError("Skeleton: Edge to node not a String");
	edge.to = Integer.parseInt((String)list.get(2));
	if(!(list.get(3) instanceof Sexp))
	    throw new ParseError("Skeleton: Edge attribute list not a Sexp");
	ListIterator iter = ((Sexp)list.get(3)).listIterator();
	while(iter.hasNext()){
	    Object next = iter.next();
	    if(!(next instanceof Sexp))
		throw new ParseError("Skeleton: Edge attribute not an Sexp");
	    parseEdgeAttribute(edge, (Sexp)next);
	}
	g.add(edge);
    }


    public static void parseNodeAttribute(Node node, Sexp list)
	throws ParseError {
	if(list.length() != 2)
	    throw new ParseError("Node: attribute not right length");
	if(!(list.get(0) instanceof String) || !(list.get(1) instanceof String))
	    throw new ParseError("Node: attribute pair not Strings");
	String att = (String)list.get(0), val = (String)list.get(1);
	if(att.equalsIgnoreCase("level")){
	    node.level = Integer.parseInt(val);
	} else if(att.equalsIgnoreCase("onclick")){
	    node.onclick = val;
	} else if(att.equalsIgnoreCase("label")){
	    node.label = val;
	} else if(att.equalsIgnoreCase("color")){
	    node.color = Colour.string2Color(val);
	} else if(att.equalsIgnoreCase("border")){
	    node.border = Colour.string2Color(val);
	} else if(att.equalsIgnoreCase("shape")){
	    node.shape = val;
	    if(node.shape.equals("rectangle") || 
	       node.shape.equals("box") || 
	       node.shape.equals("square")) node.type = Node.RULE;
	} else throw new ParseError("Node: attribute not recognized: " + att);
    }

    public static void parseEdgeAttribute(Edge edge, Sexp list)
	throws ParseError {
	if(list.length() != 2)
	    throw new ParseError("Edge: attribute not right length");
	if(!(list.get(0) instanceof String) || !(list.get(1) instanceof String))
	    throw new ParseError("Edge: attribute pair not Strings");
	String att = (String)list.get(0), val = (String)list.get(1);
	if(att.equalsIgnoreCase("label")){
	    edge.label = val;
	} else if(att.equalsIgnoreCase("color")){
	    edge.color = Colour.string2Color(val);
	} else throw new ParseError("Edge: attribute not recognized: " + att);
    }

}
