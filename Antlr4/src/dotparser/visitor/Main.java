package dotparser.visitor;

import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.tree.*;

import java.io.InputStream;
import java.io.FileInputStream;

import dotparser.antlr4.*;

public class Main {


    public static void main(String[] args) throws Exception {
        String inputfile = null;
        if(args.length > 0) inputfile = args[0];
        if(inputfile != null){
            DotParser parser = Visitor.parse(inputfile);
            if(parser != null){
                ParseTree tree = parser.graph();
                System.out.println(tree.toStringTree(parser));
            }
        } 
    }


}



