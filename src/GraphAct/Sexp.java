package GraphAct;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.StringTokenizer;
import java.util.ListIterator;

/* A non-atomic Lisp List.   (Only called Sexp because I can't    */
/* call them Lists.) Elements are either Strings or Sexps.        */

public class Sexp extends ArrayList {

    public Sexp(){ super(); }

    public Sexp(Collection collection){ super(collection); }

    public Sexp(int capacity){ super(capacity); }

    public Sexp(Sexp copy){ super(copy); }

    public Sexp(String string) throws ParseError {
	this(ParseSexp(string));
    }

    public static Sexp ParseSexp(String string) throws ParseError {
	StringTokenizer parser = 
	    new StringTokenizer(string, Parser.whitespace + Parser.delims, true);
	Sexp retval = new Sexp();
	if(!parser.hasMoreTokens()) return null;
	String tok = Parser.nextToken(parser);
	if(!tok.equals("(")) throw new ParseError("ParseSexp: didn't start with ( -- " + tok);
	ParseList(parser, retval);
	return retval;
    }

    public static Sexp ParseList(StringTokenizer parser, Sexp retval)
	throws ParseError {
	if(!parser.hasMoreTokens()) throw new ParseError("ParseList ran out of tokens");
	String tok = Parser.nextToken(parser);
	if(tok.equals("(")){
	    Sexp element = ParseList(parser, new Sexp());
	    retval.add(element);
	    return ParseList(parser, retval);
	} else if(tok.equals(")")){
	    return retval;
	} else {
	    retval.add(tok);
	    return ParseList(parser, retval);  
	}
    }
    
    int length(){ return this.size(); }

    public String toString(){
	String retval = "(";
	ListIterator iter = this.listIterator();
	while(iter.hasNext()){
	    retval += iter.next().toString();
	    if(iter.hasNext()) retval += " ";
	}
	retval += ")";
	return retval;
    }


    public static void main(String[] args){
	IO.err.println(new Sexp(Arrays.asList(args)));
	try{
	    IO.err.println(new Sexp("((1 2 3) (4 5) 6)"));
	}catch(ParseError pe){ IO.err.println(pe); }

    }

}
