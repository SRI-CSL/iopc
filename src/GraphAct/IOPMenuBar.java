package GraphAct;

import javax.swing.JMenuBar;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.KeyStroke;

import java.util.ArrayList;

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.Toolkit;

public class IOPMenuBar extends JMenuBar {

    IOPMenuBar(String description){
	super();
	Sexp list;
	try{
	    list = new Sexp(description);
	    parse(this, list);
	}catch(ParseError pe){ IO.err.println(pe); }
    }
    
    IOPMenuBar(Sexp list){
	super();
	try{
	    parse(this, list);
	}catch(ParseError pe){ IO.err.println(pe); }
    }

    public void parse(IOPMenuBar self, Sexp list) throws ParseError {
	if(list.length() == 0)
	    throw new ParseError("IOPMenuBar: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("IOPMenuBar: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("MenuBar"))
	    throw new ParseError("IOPMenuBar: didn't start with \"MenuBar\"");
	for(int i = 1; i < list.length(); i++){
	    IOPMenu menu = new IOPMenu((Sexp)list.get(i));
	    add(menu);
	}
    }
}


class IOPMenu extends JMenu implements GraphActThing {
    
    IOPMenu(Sexp list){
	super();
	try {
	    parse(this, list);
	}catch(ParseError pe){ IO.err.println(pe); }
    }

    public String getGraphActLabel(){ return getText(); }
    public void setGraphActLabel(String text){ setText(text); }

    public void parse(IOPMenu self, Sexp list) throws ParseError {
	if(list.length() < 2)
	    throw new ParseError("IOPMenu: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("IOPMenu: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("Menu"))
	    throw new ParseError("IOPMenu: didn't start with \"Menu\"");
	if(!(list.get(1) instanceof Sexp))
	    throw new ParseError("IOPMenu: label attribute not a Sexp");
	Parser.parseLabel(self, (Sexp)list.get(1));
	for(int i = 2; i < list.length(); i++){
	    IOPMenuItem menuItem = new IOPMenuItem((Sexp)list.get(i));
	    add(menuItem);
	}
    }
}


class IOPMenuItem extends JMenuItem implements GraphActThing, ActionListener {
    static final protected int menuMask = Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();    
    protected String docstring = "";
    protected String shortcut = "";
    protected String action;

    IOPMenuItem(Sexp list){
	super();
 	try {
	    parse(this, list);
	    this.addActionListener(this);
	    if(!docstring.equals("")) 
		this.setToolTipText(docstring);
	    if(!shortcut.equals("")) 
		this.setAccelerator(KeyStroke.getKeyStroke(shortcut.charAt(0), menuMask));
	}catch(ParseError pe){ IO.err.println(pe); }
    }

    public void actionPerformed(ActionEvent ae){
	String arg = (String)ae.getActionCommand();
	if(arg.equals(this.getGraphActLabel()))
	    ActorMsg.sendActorMsg(IO.out, action);
    }

    public String getGraphActLabel(){ return getText(); }
    public void setGraphActLabel(String text){ setText(text); }
    
    public void parse(IOPMenuItem self, Sexp list) throws ParseError {
	if(list.length() != 5)
	    throw new ParseError("IOPMenuItem: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("IOPMenuItem: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("MenuItem"))
	    throw new ParseError("IOPMenuItem: didn't start with \"MenuItem\"");
	if(!(list.get(1) instanceof Sexp))
	    throw new ParseError("IOPMenuItem: label attribute not a Sexp");
	Parser.parseLabel(self, (Sexp)list.get(1));
	for(int i = 2; i < 5; i++){
	    if(!(list.get(i) instanceof Sexp) )
		throw new ParseError("IOPMenuItem: " + i + "th attribute not a Sexp");
	    Sexp att = (Sexp)list.get(i);
	    if(att.length() != 2)
		throw new ParseError("IOPMenuItem: " + i + "th attribute not correct length");
	    if(!(att.get(0) instanceof String) || !(att.get(1) instanceof String))
		throw new ParseError("IOPMenuItem: " + i + "th attribute not correct form");
	    setAttribute((String)att.get(0), (String)att.get(1));
	}
    }
    
    public void setAttribute(String att, String val) throws ParseError{
	if(att.equalsIgnoreCase("docstring"))
	    this.docstring = val;
	else if(att.equalsIgnoreCase("shortcut"))
	    this.shortcut = val;
	else if(att.equalsIgnoreCase("action"))
	    this.action = val;
	else 
	    throw new ParseError("IOPMenuItem: unknown attribute " + att);
    }

}
