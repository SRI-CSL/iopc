package GraphAct;

import java.awt.Image;
import java.awt.Color;
import java.awt.MediaTracker;
import java.awt.Toolkit;
import java.awt.Dimension;
import java.awt.BorderLayout;
import java.awt.event.KeyEvent;
import java.awt.event.KeyAdapter;

import javax.swing.JFrame;

import java.io.File;

import Ezd.EzdGlyph;
import Ezd.EzdImage;
import Ezd.EzdView;   
import Ezd.EzdDrawing;


public class Sprite extends EzdGlyph implements GraphActPeon {
    private static final boolean DEBUG = false;

    private JFrame frame              = new CJFrame();
    private MediaTracker mediaTracker = new MediaTracker(frame);
    private Toolkit toolkit           = Toolkit.getDefaultToolkit();
    private String label              = "default";
    private String container          = "unknown";
    private String imageDir           = "Neko";
    private int width                 = 30;
    private int height                = 30;
    private int xPos                  = 100;
    private int yPos                  = 100;
    private String view               = "AWAKE";
    

    private String[] files;
    private String[] views;
    private Image[]  images;

    /*
     (sprite-name set <att> <val>)
     (sprite-name sleep <n millseconds>)
     (sprite-name repaint)
     (sprite-name get cust <att>)  ===> "maude\ngraphics\ncust label <att> <val>"
     (sprite-name get <cust> <att1>  ... <attN>)  
      ===> 
     "maude\ngraphics\n<cust> <container> sprite-name getReply <att1> <val1> ... <attK> <valK>"
   */

    //GraphActPeon Interface 
    public void evaluate(String[] cmd){
	if(DEBUG){
	    IO.err.println("Sprite " + this.label + " got cmd:");
	    for(int i = 0; i < cmd.length; i++)
		IO.err.println("\tcmd[" + i + "] = " + cmd[i]);
	}
	if((cmd.length == 4) && cmd[1].equalsIgnoreCase("set")){
	    setAttribute(cmd[2], cmd[3]); 
	} else if((cmd.length >= 4) && cmd[1].equalsIgnoreCase("get")){
	    String reply = "\nmaude\ngraphics\n";
	    reply +=  " " + cmd[2] + " " + container + " " + cmd[0] + " getReply";
	    for(int i = 3; i <  cmd.length; i++){
		reply +=  " " +  cmd[i] + " " + getAttribute(cmd[i]);
	    }
	    ActorMsg.sendActorMsg(IO.out, reply);
	    if(DEBUG)ActorMsg.sendActorMsg(IO.err, reply);
	} else if((cmd.length == 3) && cmd[1].equalsIgnoreCase("sleep")){
	    GraphActController.pause(Integer.parseInt(cmd[2]));
	} else if((cmd.length == 2) && cmd[1].equalsIgnoreCase("repaint")){
	    this.draw();
	} else 
	    IO.err.println("Sprite didn't understand command"); 
    }

    private void setAttribute(String att, String val){
	switch(att.charAt(0)){
	case 'l': this.label = val; break;
	case 'i': this.imageDir = val; break;
	case 'w': {
	    this.width = Integer.parseInt(val); 
	    break;
	}
	case 'h': { 
	    this.height = Integer.parseInt(val); 
	    break;
	}
	case 'x': this.xPos = Integer.parseInt(val); break;
	case 'y': this.yPos = Integer.parseInt(val); break;
	case 'v': this.view = val; break;
	default: IO.err.println("Sprite didn't understand command");
	}
    }

    private String getAttribute(String att){
	switch(att.charAt(0)){
	case 'l': return this.label;
	case 'i': return this.imageDir;
	case 'w': return "" + this.width;
	case 'h': return "" + this.height;
	case 'x': return "" + this.xPos;
	case 'y': return "" + this.yPos;
	case 'v': return this.view;
	default: IO.err.println("Sprite didn't understand command");
	}
	return null;
    }

    public String toString(){
	return "(sprite " + label + " " + imageDir + " " + view + ")";
    }
    
    public void draw(){
	int index = getIndex(view);
	if(index >= 0)
	    drawAs(new EzdImage(images[index], xPos, yPos));
    }

    private int getIndex(String view){
	int index = -1;
	for(int i = 0; i < views.length; i++)
	    if((views[i] != null) && views[i].equals(view)) return i;
	return -1;
    }

    private void initialize(){
	File iDir = new File(imageDir);
	files = iDir.list();
	if(files == null) return;
	views = new String[files.length];
	images = new Image[files.length];
	for(int i = 0; i < files.length; i++){
	    views[i] = files[i].substring(0 , files[i].indexOf('.'));
	    images[i] = toolkit.getImage(imageDir + File.separator + files[i]);
	    mediaTracker.addImage(images[i], i, width, height);
	    if(DEBUG){
		IO.err.println("files[" + i + "] = " + files[i]);
		IO.err.println("views[" + i + "] = " + views[i]);
	    }
	}
	try {
	    mediaTracker.waitForAll();
	}catch(Exception e){ IO.err.println(e); }
	if(mediaTracker.isErrorAny())
	    if(DEBUG)IO.err.println("There were errors loading some images");
	else 
	    if(DEBUG)IO.err.println("There were *no* errors loading the images");
	    
    }

    //GraphActThing Interface
    public String  getGraphActLabel(){ return label; }
    public void    setGraphActLabel(String label){ this.label = label; }

    Sprite(){
	EzdView view = new EzdView();
	EzdDrawing  drawing = new EzdDrawing();
	initialize();
	drawing.add(this);
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
	frame.setVisible(true);
	this.draw();
    }

    Sprite(Sexp list){
	try{
	    parse(this, list);
	}catch(ParseError pe){ IO.err.println(pe); }
	initialize();
    }

    Sprite(Sexp list, JFrame frame, String container){
	this.frame = frame;
	this.container = container;
	try{
	    parse(this, list);
	}catch(ParseError pe){ IO.err.println(pe); }
	if(DEBUG)IO.err.println(this);
	initialize();
	this.draw();
    }
    /*
    (sprite 
     (label  <name>)
     (images <directory>)
     (width  <width>)
     (height <height>)
     (xpos   <xpos>)
     (ypos   <ypos>)
     (view   <imagefilename>) %no format extension
     )
    */
    private void parse(Sprite self, Sexp list) throws ParseError {
	if(list.length() != 8)
	    throw new ParseError("Sprite: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Sprite: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("Sprite"))
	    throw new ParseError("Sprite: didn't start with \"Sprite\"");
	for(int i = 1; i < list.length(); i++){
	    parseAttribute(self, list.get(i));
	}
    }

    private void parseAttribute(Sprite self, Object obj) throws ParseError {
	if(!(obj instanceof Sexp))
	    throw new ParseError("Sprite Attribute: not an Sexp");
	Sexp list = (Sexp)obj;
	if(list.length() != 2)
	    throw new ParseError("Sprite Attribute: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Sprite Attribute: attribute name not a String");
	if(!(list.get(1) instanceof String))
	    throw new ParseError("Sprite Attribute: attribute value not a String");
	String att = ((String)list.get(0)).toLowerCase();
	String val = ((String)list.get(1));
	setAttribute(att, val);
    }

    public static void  main(String[] args){
	Sprite s = new Sprite();
    }
}
