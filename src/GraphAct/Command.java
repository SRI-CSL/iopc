package GraphAct;

public class Command implements GraphActThing { 
    String label;
    Object[] cmds;  //each element is a String[] =  {id,  methodName, arg1, arg2, ... argN} 

    public String getGraphActLabel(){ return label; }
    public void setGraphActLabel(String label){ this.label = label; }

    public String[][] getCommands(){
	String[][] retval = new String[cmds.length][];
	for(int i = 0; i  < cmds.length; i++){
	    Object[] arr = (Object[])cmds[i];
	    retval[i] = new String[arr.length];
	    for(int j = 0; j < arr.length; j++)
		retval[i][j] = (String)arr[j];
	}
	return retval;
    }

    static String test = 
	"(command grid123 (rover move) (rover move) (rover swing N) (rover swing S) (rover swing E)(rover swing W) (rover rotate S) (rover rotate W) (rover rotate N) (rover rotate E)(rover move) (rover move)(rover rotate S)(rover move) (rover move)(rover rotate W)(rover move) (rover move)(rover move) (rover move) (rover rotate N)(rover move) (rover move)(rover swing N))";

    public Command(){
	this(test);
    }

    public Command(String string){
	Sexp list;
	try{
	    list = new Sexp(string);
	    if(list.length() < 3) 
		throw new ParseError("Command: not long enough");
	    if(!(list.get(0) instanceof String))
		throw new ParseError("Command: didn't start with a String");
	    if(!((String)list.get(0)).equalsIgnoreCase("Command"))
		throw new ParseError("Command: didn't start with \"Command\"");
	    if(!(list.get(1) instanceof String))
		throw new ParseError("Command: label not a String");
	    label = (String)list.get(1);
	    int len = list.length();
	    cmds = new Object[len - 2];
	    for(int i = 2; i < len; i++){
		cmds[i - 2] = ((Sexp)list.get(i)).toArray();

	    }
	}catch(ParseError pe){ IO.err.println(pe); }
    }
        
    public static void main(String[] args){
	    Command cmd = new Command();
	    if(cmd == null)
		IO.err.println("null");
	    else
		IO.err.println(cmd.label);
    }
    
}

