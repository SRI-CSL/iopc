package GraphAct;

import java.awt.Color;
import java.awt.BorderLayout;
import java.awt.Rectangle;
import java.awt.event.KeyEvent;
import java.awt.event.KeyAdapter;

import javax.swing.JScrollPane;

import java.util.ArrayList;
import java.util.ListIterator;

import Ezd.EzdView;   
import Ezd.EzdDrawing;

public class PfdCanvas extends GraphActController implements GraphActWindowThing, Displayable {
    private       String      label;
    private       String      pname;
    private       Color       background  = Color.white;
    private       int         width;
    private       int         height;
    private final CJFrame     frame       = new CJFrame();
    private final EzdView     view        = new EzdView();
    private final JScrollPane scrollPane  = new JScrollPane(view);
    private final EzdDrawing  drawing     = new EzdDrawing();
    private final ArrayList   pfdList     = new ArrayList();

    CJFrame getFrame(){ return frame; }

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

    //Displayable interface
    public void addToDrawing(EzdDrawing drawing){
	ListIterator iter =  pfdList.listIterator();
	while(iter.hasNext()){
	    Pfd pfd = (Pfd)iter.next();
	    pfd.addToDrawing(drawing);
	}
    }


    public Rectangle getBounds(){
	Rectangle rect = new Rectangle();
	ListIterator iter =  pfdList.listIterator();
	while(iter.hasNext()){
	    Pfd pfd = (Pfd)iter.next();
	    rect.add(pfd.getBounds());
	}
	return rect;
    }

    
    public void extend(PfdExtension pfdExtension){
	ListIterator iter = pfdExtension.pfdList.listIterator();
	while(iter.hasNext()){
	    Pfd pfd = (Pfd)iter.next();
	    this.pfdList.add(pfd);
	    pfd.addToDrawing(drawing);
	}
	this.resizeView();
	this.refresh();
	view.repaint();
    }

    public void delete(PfdDeletion pfdDeletion){
	drawing.removeAll();
	ListIterator iter = pfdDeletion.pfdLabelList.listIterator();
	while(iter.hasNext()){
	    String pfdLabel = (String)iter.next();
	    //delete the pfd with this label from the pfdList
	    for(int i = 0; i < this.pfdList.size(); i++){
		Pfd pfd = (Pfd)this.pfdList.get(i);
		if(pfdLabel.equals(pfd.getGraphActLabel())){ 
		    this.pfdList.remove(i);
		    break;
		}
	    }
	}
	this.addToDrawing(drawing);
	this.resizeView();
	this.refresh();
	view.repaint();
    }
    
    public void refresh(){
	ListIterator iter = pfdList.listIterator();
	while(iter.hasNext())((Pfd)iter.next()).refresh();
    }
    
    public void resizeView(){
	Rectangle box = this.getBounds();
	int width = Math.max(this.width, box.width);
	int height = Math.max(this.height, box.height + 100);
	view.setSize(width, height);
    }


    PfdCanvas(String string){
	Sexp list;
	try{
	    list = new Sexp(string);
	    parse(this, list);
	}catch(ParseError pe){ IO.err.println(pe); }
	view.add(drawing);
	view.setBackground(background);
	frame.setTitle(pname);

	if((width != 0) && (height != 0)){
	    frame.setSize(width, height);
	}
	

	resizeView();
	
	frame.getContentPane().setLayout(new BorderLayout());
	frame.getContentPane().add(this.scrollPane, BorderLayout.CENTER);

	view.addKeyListener(new KeyAdapter(){
		public void keyPressed(KeyEvent ke){
		    if(ke.getKeyCode() == KeyEvent.VK_Q)frame.dispose();
		}
	    });
	this.addToDrawing(this.drawing);
	view.repaint();
	refresh();
    }

    /*

(pfdcanvas
  (label <string>)
  (pname <string>)
  (pfdlist (pfd ...)
           ...
           (pfd ...))
  (atts (background <color>) (width <num>) (height <num>) ... )
  [optional menu bar]
)

    */

    private static void parse(PfdCanvas self, Sexp list) throws ParseError { 
	if((list.length() != 5) && (list.length() != 6))
	    throw new ParseError("PfdCanvas: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("PfdCanvas: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("pfdcanvas"))
	    throw new ParseError("PfdCanvas: didn't start with \"pfdcanvas\"");
	if(!(list.get(1) instanceof Sexp))
	    throw new ParseError("PfdCanvas: label slot not a Sexp");
	Parser.parseLabel(self, (Sexp)list.get(1));
	if(!(list.get(2) instanceof Sexp))
	    throw new ParseError("PfdCanvas: pname slot not a Sexp");
	parsePname(self, (Sexp)list.get(2));
	if(!(list.get(3) instanceof Sexp))
	    throw new ParseError("PfdCanvas: pfdList slot not a Sexp");
	parsePfdList(self, (Sexp)list.get(3));
	if(!(list.get(4) instanceof Sexp))
	    throw new ParseError("PfdCanvas: atts slot not a Sexp");
	parseAtts(self, (Sexp)list.get(4));
	if(list.length() == 6){
	    IOPMenuBar iopMenuBar = new IOPMenuBar((Sexp)list.get(5));
	    self.getFrame().setJMenuBar(iopMenuBar);
	}
    }


    private static void parsePname(PfdCanvas self, Object obj) throws ParseError {
	if(!(obj instanceof Sexp))throw new ParseError("PfdCanvas:  pname not an Sexp");
	Sexp list = (Sexp)obj;
	if(list.length() != 2)
	    throw new ParseError("PfdCanvas: pname slot not right length");
   	if(!(list.get(0) instanceof String))
	    throw new ParseError("PfdCanvas: pname slot didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("pname"))
	    throw new ParseError("PfdCanvas: pname slot didn't start with \"pname\"");
   	if(!(list.get(1) instanceof String))
	    throw new ParseError("PfdCanvas: pname value not a String");
	self.pname = (String)list.get(1);
    }

    private static void parsePfdList(PfdCanvas self, Object obj) throws ParseError {
	if(!(obj instanceof Sexp))throw new ParseError("PfdCanvas:  pfdlist slot not an Sexp");
	Sexp list = (Sexp)obj;
	if(list.length() == 0)
	    throw new ParseError("PfdCanvas: pfdlist slot not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("PfdCanvas: pfdlist slot didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("pfdlist"))
	    throw new ParseError("PfdCanvas: pfdlist slot didn't start with \"pfdlist\"");
	for(int i = 1; i < list.length(); i++){
	    if(!(list.get(i) instanceof Sexp))
		throw new ParseError("PfdCanvas: pfdlist element " + i + " not a Sexp");
	    Sexp element = (Sexp)list.get(i);
	    Pfd pfd = new Pfd(element, self.getFrame());
	    self.pfdList.add(pfd);
	}
    }

    private static void parseAtts(PfdCanvas self, Object obj) throws ParseError {
	if(!(obj instanceof Sexp))throw new ParseError("PfdCanvas:  pname not an Sexp");
	Sexp list = (Sexp)obj;
	if(list.length() == 0)
	    throw new ParseError("PfdCanvas: atts slot not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("PfdCanvas: atts slot didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("atts"))
	    throw new ParseError("PfdCanvas: atts slot didn't start with \"atts\"");
	for(int i = 1; i < list.length(); i++){
	    if(!(list.get(i) instanceof Sexp))
		throw new ParseError("PfdCanvas: atts element " + i + " not a Sexp");
	    Sexp element = (Sexp)list.get(i);
	    if(element.length() != 2)
		throw new ParseError("PfdCanvas: atts element " + i + " not right length");
	    if(!(element.get(0) instanceof String))
		throw new ParseError("PfdCanvas: atts element " + i + " didn't start with a String");
	    if(((String)element.get(0)).equalsIgnoreCase("background")){
		self.background = Colour.string2Color((String)element.get(1));
	    } else if(((String)element.get(0)).equalsIgnoreCase("width")){
		self.width = Integer.parseInt((String)element.get(1));
	    } else if(((String)element.get(0)).equalsIgnoreCase("height")){
		self.height = Integer.parseInt((String)element.get(1));
	    } else continue;
	}
    }
   
/*
(pfd
  (label <string>)
  (pname <string>)
  (spine (<box1> ... <boxk>))
  (bindings ((<box1> <label> <pos>) ... ))     *** pos is top means above spine
  (connections ((<label1a> <label1b>) ... ))   *** bottom means below spine
  (atts ...)
 )

(box
  (label <string>)
  (pname <string>)
  (atts (color <color>)
        (onclick <msg>)
        (modifiers ((<box1> <pos>) ...))    <pos> is top or bottom
  )
)
*/

	

    protected static String testString = 
	"(pfdcanvas" +
	" (label testCanvas)" +
	" (pname \"PDFCanvas 01: Activated PKCz\")" +
	" (pfdlist " +
	Pfd.pfdString1 +
	" )" +
	" (atts (background white) (width 600) (height 400))" +
	" (menubar" +
	"  (menu (label file) " + 
	"   (menuitem " +
	"    (label quit) " +
	"    (action \"maude\nviewer\nqids-for-maude\n\")" +
	"    (docstring Q) " +
	"    (shortcut Q) " +
	"    )" +
	"   )" +
	"  (menu (label rules) " + 
	"   (menuitem " +
	"    (label raf1) " +
	"    (action \"maude\nviewer\nqids-for-maude\n\")" +
	"    (docstring R) " +
	"    (shortcut R) " +
	"    )" +
	"   )" +
	"  )" +
	" )";

    public static void main(String[] args){
	PfdCanvas test = new PfdCanvas(testString);
	test.display();
    }
}


/*
(pfdextend
  (label <string>)
   (pfdlist (pfd ...) ... (pfd ...))
)
*/

class PfdExtension  implements GraphActThing {
    private         String      label;
    private         PfdCanvas   pfdCanvas;
    protected final ArrayList   pfdList     = new ArrayList();

    protected static String pfdextendString1 =
	"(pfdextend  (label testCanvas) (pfdlist "  + Pfd.pfdString2 + "))";

    protected static String pfdextendString2 =
	"(pfdextend  (label testCanvas) (pfdlist "  + Pfd.pfdString3 + "))";



    //GraphActThing Interface
    public String getGraphActLabel(){ return label; }
    public void   setGraphActLabel(String label){ this.label = label; }

    PfdExtension(String string){
	Sexp list;
	try{
	    list = new Sexp(string);
	    parse(this, list);
	}catch(ParseError pe){ IO.err.println(pe); }
    }

    private static void parse(PfdExtension self, Sexp list) throws ParseError { 
	if((list.length() != 3))
	    throw new ParseError("PfdExtension: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("PfdExtension: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("pfdextend"))
	    throw new ParseError("PfdExtension: didn't start with \"pfdextend\"");
	if(!(list.get(1) instanceof Sexp))
	    throw new ParseError("PfdExtension: label slot not a Sexp");
	Parser.parseLabel(self, (Sexp)list.get(1));
	GraphActWindowThing thing = Main.getGraphActWindowThing(self.getGraphActLabel());
	if((thing == null) || !(thing instanceof PfdCanvas))
	    throw new ParseError("PfdExtension: label not one of a PfdCanvas");
	self.pfdCanvas = (PfdCanvas)thing;
	
	if(!(list.get(2) instanceof Sexp))
	    throw new ParseError("PfdExtension: pdflist slot not a Sexp");
	parsePfdList(self, (Sexp)list.get(2));
    }

    private static void parsePfdList(PfdExtension self, Object obj) throws ParseError {
	if(!(obj instanceof Sexp))throw new ParseError("PfdExtension: pfdlist slot not an Sexp");
	Sexp list = (Sexp)obj;
	if(list.length() == 0)
	    throw new ParseError("PfdExtension: pfdlist slot not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("PfdExtension: pfdlist slot didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("pfdlist"))
	    throw new ParseError("PfdExtension: pfdlist slot didn't start with \"pfdlist\"");
	for(int i = 1; i < list.length(); i++){
	    if(!(list.get(i) instanceof Sexp))
		throw new ParseError("PfdExtension: pfdlist element " + i + " not a Sexp");
	    Sexp element = (Sexp)list.get(i);
	    Pfd pfd = new Pfd(element, self.pfdCanvas.getFrame());
	    self.pfdList.add(pfd);
	}
    }
}

/*
(pfddelete
   (label <string>)
   (pfdlabellist <label> ... <label>)
)
*/

class PfdDeletion  implements GraphActThing {
    private           String      label;
    protected         PfdCanvas   pfdCanvas;
    protected final   ArrayList   pfdLabelList  = new ArrayList();

    protected static String pfddeleteString =
	"(pfddelete  (label testCanvas) (pfdlabellist PKCz-act-1))";

    // MaudeThing Interface
    public String getGraphActLabel(){ return label; }
    public void   setGraphActLabel(String label){ this.label = label; }

    PfdDeletion(String string){
	Sexp list;
	try{
	    list = new Sexp(string);
	    parse(this, list);
	}catch(ParseError pe){ IO.err.println(pe); }
    }

    private static void parse(PfdDeletion self, Sexp list) throws ParseError { 
	if((list.length() != 3))
	    throw new ParseError("PfdDeletion: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("PfdDeletion: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("pfddelete"))
	    throw new ParseError("PfdDeletion: didn't start with \"pfddelete\"");
	if(!(list.get(1) instanceof Sexp))
	    throw new ParseError("PfdDeletion: label slot not a Sexp");
	Parser.parseLabel(self, (Sexp)list.get(1));
	GraphActWindowThing thing = Main.getGraphActWindowThing(self.getGraphActLabel());
	if((thing == null) || !(thing instanceof PfdCanvas))
	    throw new ParseError("PfdExtension: label not one of a PfdCanvas");
	self.pfdCanvas = (PfdCanvas)thing;
	if(!(list.get(2) instanceof Sexp))
	    throw new ParseError("PfdDeletion: pdflist slot not a Sexp");
	parsePfdLabelList(self, (Sexp)list.get(2));
    }

    private static void parsePfdLabelList(PfdDeletion self, Object obj) throws ParseError {
	if(!(obj instanceof Sexp))throw new ParseError("PfdDeletion: pfdlist slot not an Sexp");
	Sexp list = (Sexp)obj;
	if(list.length() == 0)
	    throw new ParseError("PfdDeletion: pfdlist slot not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("PfdDeletion: pfdlist slot didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("pfdlabellist"))
	    throw new ParseError("PfdDeletion: pfdlist slot didn't start with \"pfdlabellist\"");
	for(int i = 1; i < list.length(); i++){
	    if(!(list.get(i) instanceof String))
		throw new ParseError("PfdDeletion: pfdlabellist element " + i + " not a String");
	    String element = (String)list.get(i);
	    self.pfdLabelList.add(element);
	}
    }
}
