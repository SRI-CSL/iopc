package GraphAct;

import java.awt.Color;
import java.awt.BorderLayout;
import java.awt.event.KeyEvent;
import java.awt.event.KeyAdapter;

import java.util.ArrayList;

import Ezd.EzdView;   
import Ezd.EzdDrawing;

public class IOPCanvas extends GraphActController implements GraphActWindowThing {
    private       String        label;
    private final CJFrame       frame = new CJFrame();
    private final EzdView        view = new EzdView();
    private final EzdDrawing  drawing = new EzdDrawing();

    //GraphActThing Interface
    public String getGraphActLabel(){ return label; }
    public void   setGraphActLabel(String label){ this.label = label; }

    //GraphActWindowThing Interface
    public void dispose(){
	frame.dispose();
    }

    public boolean isVisible(){
	return frame.isVisible();
    }

    public void display(){ frame.setVisible(true); };


    IOPCanvas(String string){
	Sexp list;
	try{
	    list = new Sexp(string);
	    parse(this, list);
	}catch(ParseError pe){ IO.err.println(pe); }
	view.add(drawing);
	view.setBackground(Color.white);
	frame.setTitle(label);
	frame.getContentPane().setLayout(new BorderLayout());
	frame.getContentPane().add("Center", view);
	view.addKeyListener(new KeyAdapter(){
		public void keyPressed(KeyEvent ke){
		    if(ke.getKeyCode() == KeyEvent.VK_Q)frame.dispose();
		}
	    });
    }

    /*
    (canvas 
     (label name)
     (sprite ...)
     (sprite ...)
     (sprite ...)
     (sprite ...)
     )
    */

    private void parse(IOPCanvas self, Sexp list) throws ParseError { 
	if(list.length() < 2)
	    throw new ParseError("IOPCanvas: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("IOPCanvas: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("Canvas"))
	    throw new ParseError("IOPCanvas: didn't start with \"Canvas\"");
	if(!(list.get(1) instanceof Sexp))
	    throw new ParseError("IOPCanvas: label attribute not a Sexp");
	Parser.parseLabel(self, (Sexp)list.get(1));
	for(int i = 2; i < list.length(); i++){
	    parseSprite(list.get(i));
	}
    }


    private void parseSprite(Object obj) throws ParseError {
	if(!(obj instanceof Sexp))
	    throw new ParseError("Canvas:  Sprite not an Sexp");
	Sexp list = (Sexp)obj;
	Sprite sprite = new Sprite(list, this.frame, this.label);
	addPeon(sprite);
	this.drawing.add(sprite);
    }
    
}
