package GraphAct;


public class Extension implements GraphActThing {
    String focus;
    Skeleton skeleton = new Skeleton();

    public String toString(){
	return "(extend \n" +
	    "(focus " + focus + ")\n" +
	    skeleton + ")\n";
    }

    public String getGraphActLabel(){ return skeleton.getGraphActLabel(); }
    public void setGraphActLabel(String label){ skeleton.setGraphActLabel(label); }

    public String getFocus(){ return focus; }
    public void setFocus(String focus){ this.focus = focus; }

    public Skeleton getSkeleton(){ return skeleton; }

    Extension(String string){
	Sexp list;
	try{
	    list = new Sexp(string);
	    Parser.parseExtension(this, list);
	}catch(ParseError pe){ IO.err.println(pe); }
    }
}
