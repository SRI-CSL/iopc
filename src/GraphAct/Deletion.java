package GraphAct;


public class Deletion implements GraphActThing {
    Skeleton skeleton = new Skeleton();

    public String toString(){
	return "(delete \n" +
	    skeleton + ")\n";
    }

    public String getGraphActLabel(){ return skeleton.getGraphActLabel(); }
    public void setGraphActLabel(String label){ skeleton.setGraphActLabel(label); }

    public Skeleton getSkeleton(){ return skeleton; }

    Deletion(String string){
	Sexp list;
	try{
	    list = new Sexp(string);
	    Parser.parseDeletion(this, list);
	}catch(ParseError pe){ IO.err.println(pe); }
    }
}
