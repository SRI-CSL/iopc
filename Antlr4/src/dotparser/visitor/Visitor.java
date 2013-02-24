package dotparser.visitor;

import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.tree.*;

import g2d.graph.*;
import g2d.swing.*;


import java.io.InputStream;
import java.io.FileInputStream;

import java.awt.geom.Point2D;
import java.awt.Color;
import java.awt.Dimension;

import dotparser.antlr4.*;
//these next two are the most relevant to grokking.
import dotparser.antlr4.DotBaseVisitor;
import dotparser.antlr4.DotParser;

import dotparser.antlr4.DotParser.IdContext;
import dotparser.antlr4.DotParser.A_listContext;
import dotparser.antlr4.DotParser.SubgraphContext;
import dotparser.antlr4.DotParser.EdgeopContext;
import dotparser.antlr4.DotParser.Node_idContext;

import java.util.List;
import java.util.HashMap;

// we start of restricting our attention to parsing the kinds of dot files we will write via makeDotInput
// then ...
//
//

public class Visitor extends DotBaseVisitor<Object>  {

    public static final boolean DEBUG = false;

    public final IOPGraph graph;
    public final boolean isNew;

    private String currentSubGraph = null;
    
    private HashMap<String, Attributes> subGraphAttributes = new HashMap<String, Attributes>();

    private Attributes globalNodeAttributes;
    private Attributes globalEdgeAttributes;
    private Attributes globalGraphAttributes;

    public static DotParser parseString(String string){
        DotParser parser  = null;
        if(string != null){
            try {
                ANTLRInputStream input = new ANTLRInputStream(string);
                DotLexer lexer = new DotLexer(input);
                CommonTokenStream tokens = new CommonTokenStream(lexer);
                parser = new DotParser(tokens);
            } catch(Exception e){
                System.err.println(e);
            }
        }
        return parser;
    }
    
    public static DotParser parseFile(String file){
        DotParser parser  = null;
        if(file != null){
            try {
                InputStream is = new FileInputStream(file);
                ANTLRInputStream input = new ANTLRInputStream(is);
                DotLexer lexer = new DotLexer(input);
                CommonTokenStream tokens = new CommonTokenStream(lexer);
                parser = new DotParser(tokens);
            } catch(Exception e){
                System.err.println(e);
            }
        }
        return parser;
    }

    public Visitor(IOPGraph graph){
        if(graph != null){
            this.graph = graph;
            this.isNew = false;
        } else {
            this.graph = new IOPGraph();
            this.isNew = true;
        }
    }
    
    private void setGlobalGraphAttributes(Attributes attributes){
        String bboxAttr = attributes.get("bb");
        if (bboxAttr != null) {
            if(DEBUG){ System.err.println("antlr.jar - Visitor.setGlobalGraphAttributes: bb = " + bboxAttr); }
            Dimension dim = DotParserUtils.parseBoundingBoxAttribute(bboxAttr);
            //parsing at the top level
            if(currentSubGraph == null){
                this.graph.setWidth(dim.getWidth());
                this.graph.setHeight(dim.getHeight()); 
            }
            if(DEBUG){ System.err.println("Visitor.setGlobalGraphAttributes: graph dimension = " + dim); }
            if(this.graph.size() > Manifold.THRESHOLD){
                if(DEBUG){ System.err.println("Visitor.setGlobalGraphAttributes: creating manifold"); }
                this.graph.createManifold();
            }
        }
        if(currentSubGraph == null){
            if(this.globalGraphAttributes == null){
                this.globalGraphAttributes = attributes;
            } else {
                // add/replace the old with the new
                this.globalGraphAttributes.putAll(attributes);
            }
        } else {
            Attributes current = subGraphAttributes.get(currentSubGraph);
            if(current != null){
                current.putAll(attributes);
            } else {
                System.err.println("This graph is crazy. No comprendo esta graph.");
            }
        }
    }
    
    private void setGlobalNodeAttributes(Attributes attributes){
        if(DEBUG){ System.err.println("globalNodeAttributes: " + attributes); }
        if(this.globalNodeAttributes == null){
            this.globalNodeAttributes = attributes;
        } else {
            // add/replace the old with the new
            this.globalNodeAttributes.putAll(attributes);
        }
    }
    
    private void setGlobalEdgeAttributes(Attributes attributes){
        if(DEBUG){ System.err.println("globalEdgeAttributes: " + attributes); }
        if(this.globalEdgeAttributes == null){
            this.globalEdgeAttributes = attributes;
        } else {
            // add/replace the old with the new
            this.globalEdgeAttributes.putAll(attributes);
        }
    }
    

    public Object visitGraph(DotParser.GraphContext ctx) { 


        Object retval = super.visitChildren(ctx);
        if(DEBUG){ System.err.println("Visited graph: " + ctx.id().getText()); }
        return  retval;


    }

    public Object visitSubgraph(DotParser.SubgraphContext ctx) { 
        String id = ctx.id().getText();
        String parentSubGraph = this.currentSubGraph;
        Attributes attributes = new Attributes();
        this.subGraphAttributes.put(id, attributes);
        //push
        this.currentSubGraph = id;
        Object retval = visitChildren(ctx); 
        //pop
        this.currentSubGraph = parentSubGraph;
        return retval;
    }
    
    
    public Object visitAttr_stmt(DotParser.Attr_stmtContext ctx) { 
        Attributes attributes = (Attributes)visit(ctx.attr_list());
        //global attributes are at top level
        if(ctx.GRAPH() != null){
            setGlobalGraphAttributes(attributes);
        } else if(ctx.NODE() != null){
            setGlobalNodeAttributes(attributes);
        } else if(ctx.EDGE() != null){
            setGlobalEdgeAttributes(attributes);
        }
        return attributes; 
    }
    
    public Object visitEdge_stmt(DotParser.Edge_stmtContext ctx) {
        Object retval = super.visitChildren(ctx); 
        String kind = "?";
        if(ctx.subgraph() != null){  
            kind = "subgraph";
        } else if(ctx.node_id() != null){  
            kind = "node_id";
        }
        EdgeRHS rhs = (EdgeRHS)visit(ctx.edgeRHS());
        String source = ctx.node_id().id().getText();
        String target = rhs.node;
        Attributes attributes = (Attributes)visit(ctx.attr_list());
        if(DEBUG){
            System.err.println("Edge: " + kind + " " + source + " " + rhs);        
            System.err.println("\tattributes: " + attributes);
        }
        //do the edge stuff here ...
        IOPEdge edge;
        IOPNode n1, n2;
        n1 = this.graph.getNode(source);
        n2 = this.graph.getNode(target);
        if(this.isNew) {
            edge = new IOPEdge(n1, n2);
            this.graph.addEdge(edge);
        } else {
            edge = this.graph.getEdge(n1, n2);
        }
        double graphHeight = this.graph.getHeight();
        boolean ok = DotParserUtils.parseEdgeAttributes(edge,  attributes, graphHeight);
        this.graph.add2Manifold(edge);
        return retval;
    }

    public Object visitEdgeRHS(DotParser.EdgeRHSContext ctx) { 
        List<SubgraphContext> subgraphs = ctx.subgraph();
        List<EdgeopContext> edgeops = ctx.edgeop();
        List<Node_idContext> nodes = ctx.node_id();
        if((nodes.size() != 1) && (edgeops.size() != 1)){
            System.err.println("EdgeRHS:");
            System.err.println("\tsubgraphs = " +  subgraphs.size());
            System.err.println("\tnodes = " +  nodes.size());
            System.err.println("\tedgeops = " +  edgeops.size());
        } else {
            //see assumption above ( edgeops == " -> ")
            if(edgeops.get(0).ARROW() != null){
                return new EdgeRHS("->", nodes.get(0).id().getText());
            }
        }
        return null; 
    }

    public Object visitEdgeop(DotParser.EdgeopContext ctx) {
        return visitChildren(ctx); 
    }

    
    public Object visitNode_stmt(DotParser.Node_stmtContext ctx) { 
        String nid = ctx.node_id().getText();
        //do the node stuff here
        try {
            Attributes attributes = (Attributes)visit(ctx.attr_list());
            double graphHeight = this.graph.getHeight();
            if(DEBUG){
                System.err.println("Node: " + nid);
                System.err.println("\tattributes: " + attributes);
            }
            IOPNode node = null;
            if(this.isNew) {
                node = DotParserUtils.parseNodeAttributes(null,  nid, attributes, graphHeight, globalNodeAttributes);
                this.graph.addNode(node);
            } else {
                node = DotParserUtils.parseNodeAttributes(this.graph.getNode(nid), nid, attributes, graphHeight, globalNodeAttributes);
            }
            this.graph.add2Manifold(node);
        } catch(Exception e){ System.err.println(e);  e.printStackTrace(java.lang.System.err); }
        return nid;
    }

    public Object visitAttr_list(DotParser.Attr_listContext ctx) { 
        List<A_listContext> list = ctx.a_list();
        //usually only one of these; but if more we'll need to join them
        Attributes attrs = null;
        for(A_listContext al : list){
            Attributes attrs0 = (Attributes)visit(al);
            if(attrs == null){
                attrs = attrs0;
            } else {
                attrs.putAll(attrs0);
            }
        }
        return attrs; 
    }

    public Object visitA_list(DotParser.A_listContext ctx) { 
        List<IdContext> list = ctx.id();
        Attributes attributes = new Attributes();
        for(int i = 0; i < list.size(); i = i + 2){
            String k = value(list.get(i));
            String v = value(list.get(i + 1));
            attributes.put(k, v);
        }
        return attributes; 
    }
    

    private String value(IdContext id){
        Object v = null;
        v = id.ID();
        if(v != null){ return v.toString(); }
        v = id.STRING();
        if(v != null){ 
            //pretty ugly place to fix Dot's brain damage
            String s = v.toString();
            return s.replaceAll("\\\\\n", ""); 
        }
        v = id.NUMBER();
        if(v != null){ return v.toString(); }
        v = id.HTML_STRING();
        if(v != null){ return v.toString(); }
        return null;
    }
    

    public static class Attributes extends HashMap<String,String> {
        public Attributes(){
            super();
        }
    }
    
    //for returning two things out of a rhs context
    public static class EdgeRHS {
        public final String node;
        public final String op;

        public EdgeRHS(String op, String node){
            this.node = node;
            this.op = op;
        }

        public String toString(){
            return op + " " + node;
        }
    }
    
    private static void testFromFile(String file){
        DotParser parser = Visitor.parseFile(file);
        test(parser);
    }

    private static void testFromString(String string){
        DotParser parser = Visitor.parseString(string);
        test(parser);
    }

    private static void test(DotParser parser){
        Visitor v = null;
        if(parser != null){
            ParseTree tree = parser.graph();
            v = new Visitor(null);
            v.visit(tree);
        }
        IOPGraph graph = v.graph;
        IOPView view = new IOPView(true, true);
        IOPFrame frame = new IOPFrame("Layout by dot, Parsing by Antl4", view);
        view.add(graph);
        frame.setVisible(true);
        view.repaint();
    }

    public static void main(String[] args){
        //testFromFile(args[0]);
        String string = g2d.util.IO.file2String(args[0]);
        //System.err.println(string);
        testFromString(string);
    }
    
}