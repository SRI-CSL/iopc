package GraphAct;

import java.io.OutputStream;

public class ActorMsg {
    public static final boolean DEBUG = false;

    public static final int UNKNOWN    = -1;
    public static final int GRAPH      = 0;
    public static final int STRING     = 1;
    public static final int EXTEND     = 2;
    public static final int DELETE     = 3;
    public static final int GRID       = 4;
    public static final int COMMAND    = 5;
    public static final int CANVAS     = 6;
    public static final int PFDCANVAS  = 7;
    public static final int PFDEXTEND  = 8;
    public static final int PFDDELETE  = 9;
    
    public static final int SIZE = 32;
    
    private String sender;
    private String body;
    private int type = UNKNOWN;
    
    ActorMsg(String msg){
	int len = msg.length();
	if((msg == null) || 
	   (len < 3) ||
	   (msg.charAt(0) != '(')) return;
	while(msg.charAt(len - 1) == '\n') len--;
	if(msg.charAt(len - 1) != ')') return;
	int space = msg.indexOf(' ');
	if(space < 0) return;
	sender = msg.substring(1, space).trim();
	body = msg.substring(space + 1, len).trim();
	if(body.indexOf("graph") == 1){
	    type = GRAPH;
	} else if(body.indexOf("string") == 1){
	    type = STRING;
	} else if(body.indexOf("extend") == 1){
	    type = EXTEND;
	} else if(body.indexOf("delete") == 1){
	    type = DELETE;
	} else if(body.indexOf("grid") == 1){
	    type = GRID;
	} else if(body.indexOf("command") == 1){
	    type = COMMAND;
	} else if(body.indexOf("canvas") == 1){
	    type = CANVAS;
	} else if (body.indexOf("pfdcanvas") == 1){
	    type = PFDCANVAS;
	} else if (body.indexOf("pfdextend") == 1){
	    type = PFDEXTEND;
	} else if (body.indexOf("pfddelete") == 1){
	    type = PFDDELETE;
	}
    }
    public String getSender(){ return sender; }
    public String getBody(){ return body; }
    public int getType(){ return type; }
   

    public static ActorMsg readActorMsg(){
	int bytesDue = readInt(), bytesLeft = bytesDue, bytesRead = 0, bytesTotal = 0;
	byte[] buff = new byte[bytesDue];
	while(bytesTotal != bytesDue){
	    try {
		bytesRead = IO.in.read(buff, bytesTotal, bytesLeft);
		bytesTotal += bytesRead;
		bytesLeft -= bytesRead;
	    }catch(Exception e){ 
		IO.err.println(e); 
		break;
	    }
	}
	if(bytesTotal != bytesDue){
	    IO.err.println("bytesTotal != bytesDue: " + bytesTotal + " " + bytesDue);
	}
	String command = new String(buff);
	if(DEBUG)
	    IO.err.println("readActorMsg read string: " + command);
	return new ActorMsg(command);
    }
 
    public static int readInt(){
	byte[] buff = new byte[SIZE];
	int i;
	try {
	    for(i = 0; i < SIZE; i++){
		IO.in.read(buff, i, 1);
		if(buff[i] == '\n') break;
	    }
	    return Integer.parseInt(new String(buff, 0, i));
	}catch(Exception e){ IO.err.println("readInt failed: " + e); }
	return 0;
    }

    public String toString(){
	String retval = "Sender = \"" + this.getSender() + "\"\n";
	retval += "Body = \"" + this.getBody() + "\"\n";
	retval += "Type = \"" + this.getType() + "\"\n";
	return retval;
    }
	

    public static void sendActorMsg(OutputStream dest, String body){
	String message = "" + body.length() + "\n" + body;
	try{
	    dest.write(message.getBytes("US-ASCII"));
	}catch(Exception e){ IO.err.println(e); }
    }
}
