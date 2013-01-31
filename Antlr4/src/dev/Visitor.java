package dev;

import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.tree.*;

import g2d.graph.*;

import antlr4.*;
import antlr4.DotParser.IdContext;

import java.util.List;

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
        System.err.println("Edge: ");
        return retval;
    }
    
    public Object visitNode_stmt(DotParser.Node_stmtContext ctx) { 
        Object id = visit(ctx.node_id());
        Object attrs = visit(ctx.attr_list());
        //        Object retval = super.visitChildren(ctx); 
        System.err.println("Node: " + ctx.node_id().getText());
        return id;
    }

    public Object visitAttr_list(DotParser.Attr_listContext ctx) { 
        System.err.println("Attribute List: ");
        Object attrs = visitChildren(ctx); 
        return attrs; 
    }

    public Object visitA_list(DotParser.A_listContext ctx) { 
        List<IdContext> list = ctx.id();
        for(IdContext id : list){
            System.err.println("id = " + value(id));
        }
        return visitChildren(ctx); 
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

}