package dev;

import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.tree.*;

import g2d.graph.*;
import g2d.swing.*;

import antlr4.*;

import java.awt.geom.Point2D;
import java.awt.Color;
import java.awt.Dimension;

//these next  two are the most relevant to grokking.
import antlr4.DotBaseVisitor;
import antlr4.DotParser;

import antlr4.DotParser.IdContext;
import antlr4.DotParser.A_listContext;
import antlr4.DotParser.SubgraphContext;
import antlr4.DotParser.EdgeopContext;
import antlr4.DotParser.Node_idContext;

import java.util.List;
import java.util.HashMap;

// we start of restricting our attention to parsing the kinds of dot files we will write via makeDotInput
// then ...
//
//

public class Visitor extends DotBaseVisitor<Object>  {
    public final IOPGraph graph;
    public final boolean isNew;

    public Visitor(IOPGraph graph){
        if(graph != null){
            this.graph = graph;
            this.isNew = false;
        } else {
            this.graph = new IOPGraph();
            this.isNew = true;
        }
    }

    public Object visitGraph(DotParser.GraphContext ctx) { 


        Object retval = super.visitChildren(ctx);
        System.err.println("Visited graph: " + ctx.id().getText());
        return  retval;


    }
    
    public Object visitAttr_stmt(DotParser.Attr_stmtContext ctx) { 
        Object attrs = visit(ctx.attr_list());
        //graph attributes are at top level
        if(ctx.GRAPH() != null && attrs instanceof Attributes){
            Attributes attributes = (Attributes)attrs;
            String bboxAttr = attributes.get("bb");
                if (bboxAttr != null) {
                   Dimension dim = DotParserUtils.parseBoundingBoxAttribute(bboxAttr);
                   this.graph.setWidth(dim.getWidth());
                   this.graph.setHeight(dim.getHeight()); 
                   System.err.println("Visitor.visitAttr_stmt: graph dimension = " + dim);
                   if(this.graph.size() > Manifold.THRESHOLD){
                       System.err.println("Visitor.visitAttr_stmt: creating manifold");
                       this.graph.createManifold();
                   }
                }
        }
        return attrs; 
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
        System.err.println("Edge: " + kind + " " + source + " " + rhs);        
        System.err.println("\tattributes: " + attributes);
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
            IOPNode node = null;
            if(this.isNew) {
                node = DotParserUtils.parseNodeAttributes(null,  nid, attributes, graphHeight);
                this.graph.addNode(node);
            } else {
                node = DotParserUtils.parseNodeAttributes(this.graph.getNode(nid),  nid, attributes, graphHeight);
            }
            this.graph.add2Manifold(node);
            System.err.println("Node: " + nid);
            System.err.println("\tattributes: " + attributes);
        } catch(Exception e){ System.err.println(e);  e.printStackTrace(java.lang.System.err); }
        return nid;
    }

    
    public Object visitAttr_list(DotParser.Attr_listContext ctx) { 
        List<A_listContext> list = ctx.a_list();
        //usually only one of these; but if more we'll need to join them
        Object entry = null;
        for(A_listContext al : list){
            entry = visit(al);
        }
        return entry; 
    }

    public Object visitA_list(DotParser.A_listContext ctx) { 
        List<IdContext> list = ctx.id();
        Attributes attributes = new Attributes();
        for(int i = 0; i < list.size(); i = i + 2){
            attributes.put(value(list.get(i)), value(list.get(i + 1)));
        }
        return attributes; 
    }
    

    private String value(IdContext id){
        Object v = null;
        v = id.ID();
        if(v != null){ return v.toString(); }
        v = id.STRING();
        if(v != null){ return v.toString(); }
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
    
    public static void main(String[] args){
        DotParser parser = DotUtil.parse(args[0]);
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
    
}