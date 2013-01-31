package dev;

import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.tree.*;

import java.io.InputStream;
import java.io.FileInputStream;

import antlr4.*;

public class DotUtil {

    public static DotParser parse(String file){
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
    
    
}



