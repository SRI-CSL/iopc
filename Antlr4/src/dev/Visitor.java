package dev;

import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.tree.*;

import g2d.graph.*;

import antlr4.*;
import antlr4.DotParser.IdContext;
import antlr4.DotParser.A_listContext;
import antlr4.DotParser.SubgraphContext;
import antlr4.DotParser.EdgeopContext;
import antlr4.DotParser.Node_idContext;

import java.util.List;
import java.util.HashMap;

// we start of just parsing the types of dot files we will write via makeDotInput
// then ...
public class Visitor extends DotBaseVisitor<Object>  {
    public final IOPGraph graph;

    public Visitor(IOPGraph graph){
        if(graph != null){
            this.graph = graph;
        } else {
            this.graph = new IOPGraph();
        }
    }

    public Object visitGraph(DotParser.GraphContext ctx) { 
        Object retval = super.visitChildren(ctx);
        System.err.println("Visited graph: " + ctx.id().getText());
        return  retval;
    }

    public Object visitEdge_stmt(DotParser.Edge_stmtContext ctx) {
        Object retval = super.visitChildren(ctx); 
        String kind = "?";
        if(ctx.subgraph() != null){  
            kind = "subgraph";
        } else if(ctx.node_id() != null){  
            kind = "node_id";
        }
        Object rhs = visit(ctx.edgeRHS());
        Object attrs = visit(ctx.attr_list());
        System.err.println("Edge: " + kind + " " + ctx.node_id().id().getText() + " " + rhs);        
        System.err.println("\tattributes: " + attrs);
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
        Object id = visit(ctx.node_id());
        Object attrs = visit(ctx.attr_list());
        System.err.println("Node: " + ctx.node_id().getText());
        System.err.println("\tattributes: " + attrs);
        return id;
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
    
    public static void main(String[] args){
        DotParser parser = DotUtil.parse(args[0]);
        if(parser != null){
            ParseTree tree = parser.graph();
            Visitor v = new Visitor(null);
            v.visit(tree);
        }
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
    

}