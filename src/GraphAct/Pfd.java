package GraphAct;

import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Point;
import java.awt.Rectangle;

import java.awt.event.MouseEvent;

import java.util.ArrayList;
import java.util.ListIterator;

import Ezd.EzdDrawing;
import Ezd.EzdView;
import Ezd.EzdGlyph;
import Ezd.EzdString;
import Ezd.EzdFillRectangle;
import Ezd.EzdRectangle;
import Ezd.EzdLine;
    
public class Pfd implements GraphActThing, Displayable  {
    protected static       Font      textSmall  = new Font( "Lucinda Sans", Font.PLAIN, 8 );
    protected static       Font      textMedium = new Font( "Lucinda Sans", Font.PLAIN, 9 );
    protected static       Font      textLarge  = new Font( "Lucinda Sans", Font.BOLD, 12 );
    public    static final int       TOP        = 0;
    public    static final int       BOTTOM     = 1;
    public    static final int       SPINE      = 0;
    public    static final int       MODIFIER   = 1;
    public    static final int       BINDING    = 2;
    private                CJFrame   frame;
    private                String    label;
    private                String    pname;
    private          final ArrayList spineGlyphs       = new ArrayList();
    private          final ArrayList connectionGlyphs  = new ArrayList();
    private          final ArrayList bindingGlyphs     = new ArrayList();
    private                int       x;
    private                int       y;
    private                PfdLabel  labelGlyph;

    // GraphActThing Interface
    public String getGraphActLabel(){ return label; }
    public void   setGraphActLabel(String label){ this.label = label; }


    //Displayable interface
    public void addToDrawing(EzdDrawing drawing){
	ListIterator iter;
	iter =  connectionGlyphs.listIterator();
	while(iter.hasNext()){
	    EzdGlyph glyph = (EzdGlyph)iter.next();
	    drawing.add(glyph);
	}
	iter = spineGlyphs.listIterator();
	while(iter.hasNext()){
	    PfdBox glyph = (PfdBox)iter.next();
	    glyph.addToDrawing(drawing);
	}
	iter = bindingGlyphs.listIterator();
	while(iter.hasNext()){
	    EzdGlyph glyph = (EzdGlyph)iter.next();
	    drawing.add(glyph);
	}
	drawing.add(this.labelGlyph);
    }


    public Rectangle getBounds(){
	Rectangle rect = new Rectangle(new Point(this.x, this.y));
	ListIterator iter;
	iter =  this.connectionGlyphs.listIterator();
	while(iter.hasNext()){
	    Displayable glyph = (Displayable)iter.next();
	    rect.add(glyph.getBounds());
	}
	iter = this.spineGlyphs.listIterator();
	while(iter.hasNext()){
	    Displayable glyph = (Displayable)iter.next();
	    rect.add(glyph.getBounds());
	}
	iter = this.bindingGlyphs.listIterator();
	while(iter.hasNext()){
	    Displayable glyph = (Displayable)iter.next();
	    rect.add(glyph.getBounds());
	}
	rect.add(this.labelGlyph.getBounds());
	return rect;
    }

    
    public PfdBox getPfdBoxByLabel(String label){
	ListIterator iter;
	iter = spineGlyphs.listIterator();
	while(iter.hasNext()){
	    PfdBox box = (PfdBox)iter.next();
	    if(box.getGraphActLabel().equalsIgnoreCase(label)) return box;
	}
	iter = bindingGlyphs.listIterator();
	while(iter.hasNext()){
	    PfdBox box = (PfdBox)iter.next();
	    if(box.getGraphActLabel().equalsIgnoreCase(label)) return box;
	}
	return null;
    }
    
    public void positionGlyphs(){
	int i = 0;
	ListIterator iter;
	iter =  spineGlyphs.listIterator();
	PfdBox pfdBox = null;
	while(iter.hasNext()){
	    pfdBox = (PfdBox)iter.next();
	    pfdBox.setX(this.x + (i * pfdBox.getWidth()));
	    pfdBox.setY(this.y);
	    i++;
	}
	this.labelGlyph.y = this.y;
	this.labelGlyph.x = this.x + ((pfdBox != null) ? (i * pfdBox.getWidth()) : 0);
	iter = bindingGlyphs.listIterator();

	while(iter.hasNext()){
	    PfdBoxBinding binding = (PfdBoxBinding)iter.next();
	    int           position = binding.getPosition(); 
	    PfdBox        site = binding.getSite();
	    if(position == Pfd.TOP){
		binding.setX(site.getX());
		binding.setY(site.getY() - 2 * site.getHeight());
	    } else if(position == Pfd.BOTTOM){
		binding.setX(site.getX());
		binding.setY(site.getY() + 2 * site.getHeight());
	    }
	}
    }
    
    public void refresh(){
	ListIterator iter;
	iter =  connectionGlyphs.listIterator();
	while(iter.hasNext()){
	    PfdConnection connection = (PfdConnection)iter.next();
	    connection.draw();
	}
	iter = spineGlyphs.listIterator();
	while(iter.hasNext()){
	    PfdBox spineGlyph = (PfdBox)iter.next();
	    spineGlyph.draw(Pfd.SPINE);
	}
	iter = bindingGlyphs.listIterator();
	while(iter.hasNext()){
	    PfdBox bindingGlyph = (PfdBox)iter.next();
	    bindingGlyph.draw(Pfd.BINDING);
	}
	this.labelGlyph.draw();
    }

    Pfd(Sexp list, CJFrame frame){
	this.frame = frame;
	try{
	    parse(this, list);
	}catch(ParseError pe){ IO.err.println(pe); }
	this.labelGlyph = new PfdLabel(this.pname, this.frame);
	positionGlyphs();
    }

/*
(pfd
  (label <string>)
  (pname <string>)
  (spine (<box1> ... <boxk>))
  (bindings ((<box1> <label> <pos>) ... ))     *** pos is top means above spine
  (connections ((<label1a> <label1b>) ... ))   *** bottom means below spine
  (atts ...).
 )

*/

    public void parse(Pfd self, Sexp list) throws ParseError {
	if(list.length() != 7) 
	    throw new ParseError("Pfd: not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Pfd: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("pfd"))
	    throw new ParseError("Pfd: didn't start with \"pfd\"");
	if(!(list.get(1) instanceof Sexp))
	    throw new ParseError("Pfd: label slot not a Sexp");
	Parser.parseLabel(self, (Sexp)list.get(1));
	if(!(list.get(2) instanceof Sexp))
	    throw new ParseError("Pfd: pname slot not a Sexp");
	parsePname(self, (Sexp)list.get(2));
	if(!(list.get(3) instanceof Sexp))
	    throw new ParseError("Pfd: spine slot not a Sexp");
	parseSpine(self, (Sexp)list.get(3));
	if(!(list.get(4) instanceof Sexp))
	    throw new ParseError("Pfd: bindings slot not a Sexp");
	parseBindings(self, (Sexp)list.get(4));
	if(!(list.get(5) instanceof Sexp))
	    throw new ParseError("Pfd: connections slot not a Sexp");
	parseConnections(self, (Sexp)list.get(5));
	if(!(list.get(6) instanceof Sexp))
	    throw new ParseError("Pfd: spine slot not a Sexp");
	parseAtts(self, (Sexp)list.get(6));
    }
    
    
    
    private void parsePname(Pfd self, Object obj) throws ParseError {
	if(!(obj instanceof Sexp))throw new ParseError("Pfd:  pname not an Sexp");
	Sexp list = (Sexp)obj;
	if(list.length() != 2)
	    throw new ParseError("Pfd: pname slot not right length");
   	if(!(list.get(0) instanceof String))
	    throw new ParseError("Pfd: pname slot didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("pname"))
	    throw new ParseError("Pfd: pname slot didn't start with \"pname\"");
   	if(!(list.get(1) instanceof String))
	    throw new ParseError("Pfd: pname value not a String");
	self.pname = (String)list.get(1);
    }
    
    private void parseSpine(Pfd self, Object obj) throws ParseError {
	if(!(obj instanceof Sexp))throw new ParseError("Pfd:  pname not an Sexp");
	Sexp list = (Sexp)obj;
	if(list.length() == 0)
	    throw new ParseError("Pfd: spine slot not right length");
     	if(!(list.get(0) instanceof String))
	    throw new ParseError("Pfd: spine slot didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("spine"))
	    throw new ParseError("Pfd: spine slot didn't start with \"spine\"");
	for(int i = 1; i < list.length(); i++){
	    if(!(list.get(i) instanceof Sexp))
		throw new ParseError("Pfd: spine element " + i + " not a Sexp");
	    Sexp element = (Sexp)list.get(i);
	    PfdBox pfdBox = new PfdBox(element, self.frame);
	    spineGlyphs.add(pfdBox);
	}
    }   

    /*
      (bindings ((<box1> <label> <pos>) ... ))     *** pos is top means above spine
    */
    private void parseBindings(Pfd self, Object obj) throws ParseError {
	if(!(obj instanceof Sexp))throw new ParseError("Pfd:  pname not an Sexp");
	Sexp list = (Sexp)obj;
	if(list.length() == 0)
	    throw new ParseError("Pfd: bindings slot not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Pfd: bindings slot didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("bindings"))
	    throw new ParseError("Pfd: bindings slot didn't start with \"bindings\"");
	for(int i = 1; i < list.length(); i++){
	    if(!(list.get(i) instanceof Sexp))
		throw new ParseError("Pfd:  binding " + i + " not an Sexp");
	    Sexp binding = (Sexp)list.get(i);
	    if(binding.length() != 3)
		throw new ParseError("Pfd: binding " + i + " not right length");
	    PfdBoxBinding bindingBox = 
		new PfdBoxBinding(self.getPfdBoxByLabel((String)binding.get(1)), 
				  (Sexp)binding.get(0), 
				  self.frame);
	    bindingBox.setPosition((String)binding.get(2));
	    self.bindingGlyphs.add(bindingBox);
	}
    }

    /*
      (connections ((<label1a> <label1b>) ... ))  
    */
    private void parseConnections(Pfd self, Object obj) throws ParseError {
	if(!(obj instanceof Sexp))throw new ParseError("Pfd:  pname not an Sexp");
	Sexp list = (Sexp)obj;
 	if(list.length() == 0)
	    throw new ParseError("Pfd: connections slot not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Pfd: connections slot didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("connections"))
	    throw new ParseError("Pfd: connections slot didn't start with \"connections\"");
 	for(int i = 1; i < list.length(); i++){
	    if(!(list.get(i) instanceof Sexp))
		throw new ParseError("Pfd:  connection " + i + " not an Sexp");
	    Sexp connection = (Sexp)list.get(i);
	    if(connection.length() != 2)
		throw new ParseError("Pfd: connection " + i + " not right length");
            PfdBox b0 = self.getPfdBoxByLabel((String)connection.get(0));
            if(b0 == null)
                throw new ParseError("Pfd: connection 0 " + self.label + 
                                     " didn't find box with label " + (String)connection.get(0));
            PfdBox b1 = self.getPfdBoxByLabel((String)connection.get(1));
            if(b1 == null)
                throw new ParseError("Pfd: connection 1 " + self.label + 
                                     " didn't find box with label " + (String)connection.get(1));

            PfdConnection connectionGlyph = 
                new PfdConnection(b0, b1);
	    self.connectionGlyphs.add(connectionGlyph);
	}
   }   

    private void parseAtts(Pfd self, Object obj) throws ParseError {
	if(!(obj instanceof Sexp))throw new ParseError("Pfd:  atts slot not an Sexp");
	Sexp list = (Sexp)obj;
	if(list.length() == 0)
	    throw new ParseError("Pfd: atts slot not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("Pfd: atts slot didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("atts"))
	    throw new ParseError("Pfd: atts slot didn't start with \"atts\"");
	for(int i = 1; i < list.length(); i++){
	    if(!(list.get(i) instanceof Sexp))
		throw new ParseError("Pfd: atts element " + i + " not a Sexp");
	    Sexp element = (Sexp)list.get(i);
	    if(element.length() != 2)
		throw new ParseError("Pfd: atts element " + i + " not right length");
	    if(!(element.get(0) instanceof String))
		throw new ParseError("Pfd: atts element " + i + " didn't start with a String");
	    if(((String)element.get(0)).equalsIgnoreCase("x")){
		self.x = Integer.parseInt((String)element.get(1));
	    } else if(((String)element.get(0)).equalsIgnoreCase("y")){
		self.y = Integer.parseInt((String)element.get(1));
	    } else continue;
	}
	
    }

    protected  static String pfdString1 =
	"  (pfd " +
        "   (label  \"PKCz-act-1\") " +
        "   (pname  \"PKCz activated\") " +
	"   (spine " +
	"     (box (label 1) (pname S43) (atts (color lightPurple) (onclick \"clicked on S43\n\"))) " +
	"     (box (label 2) (pname RBD) (atts (color blue) (onclick \"clicked on RBD\n\"))) " +
	"     (box (label 3) (pname C1) (atts (color blue) (onclick \"clicked on C1\n\"))) " +
	"     (box (label 4) (pname S259) (atts (color C:RGB:156:0:156:125 ) (onclick \"clicked on S259\n\") (modifiers (((box (label 4-1) (pname P) (atts (color purple) (onclick \"clicked on S259's modifier\n\")))  bottom)))))" +
	"     (box (label 5) (pname S338) (atts (color purple) (onclick \"clicked on S338\n\")))" +
	"     (box (label 6) (pname Y341) (atts (color purple) (onclick \"clicked on Y341\n\")))" +
	"     (box (label 7) (pname PABM) (atts (color lightBlue) (onclick \"clicked on PABM\n\")))" +
	"     (box (label 8) (pname S612) (atts (color purple) (onclick \"clicked on S621\n\") (modifiers (((box (label 8-1) (pname P) (atts (color purple) (onclick \"clicked on S612's modifier\n\")))  bottom)))))" +
        "   ) " +
	"   (bindings ((box (label 4b) (pname \"SBD\") (atts (color green) (onclick \"clicked on SBD[14-3-3]\n\"))) 4 bottom)((box (label 8b) (pname \"SBD\") (atts (color green) (onclick \"clicked on SBD[14-3-3]\n\"))) 8 bottom))" +
	"   (connections (4b 8b) (4 4b) (8 8b))" +
        "   (atts (x 10) (y 110))" +
        "  )";

   protected  static String pfdString2 =
	"  (pfd " +
        "   (label  \"PKCz-act-2\") " +
       "   (pname  \"\") " + //PKCz activated
	"   (spine " +
	"     (box (label 1) (pname S43) (atts (color lightPurple) (onclick \"clicked on S43\n\"))) " +
	"     (box (label 2) (pname RBD) (atts (color  C:RGB:0:0:255:200) (onclick \"clicked on RBD\n\"))) " +
	"     (box (label 3) (pname C1) (atts (color C:RGB:0:0:255:150) (onclick \"clicked on C1\n\"))) " +
	"     (box (label 4) (pname S259) (atts (color C:RGB:0:0:255:100) (onclick \"clicked on S259\n\") (modifiers (((box (label 4-1) (pname P) (atts (color purple) (onclick \"clicked on S259's modifier\n\")))  top)))))" +
	"     (box (label 5) (pname S338) (atts (color purple) (onclick \"clicked on S338\n\")))" +
	"     (box (label 6) (pname Y341) (atts (color purple) (onclick \"clicked on Y341\n\")))" +
	"     (box (label 7) (pname PABM) (atts (color lightBlue) (onclick \"clicked on PABM\n\")))" +
	"     (box (label 8) (pname S612) (atts (color purple) (onclick \"clicked on S621\n\") (modifiers (((box (label 8-1) (pname P) (atts (color purple) (onclick \"clicked on S612's modifier\n\")))  top)))))" +
        "   ) " +
	"   (bindings  " + 
               "((box (label 4b) (pname \"SBD\") (atts (color green) (onclick \"clicked on SBD[14-3-3]\n\"))) 4 top)" + 
               "((box (label 8b) (pname \"SBD\") (atts (color green) (onclick \"clicked on SBD[14-3-3]\n\"))) 8 top))" +
	"   (connections (4b 8b) (4 4b) (8 8b))" +
        "   (atts (x 10) (y 220))" +
       "  )";

   protected  static String pfdString3 =
	"  (pfd " +
        "   (label  \"PKCz-act-3\") " +
        "   (pname  \"PKCz activated\") " +
	"   (spine " +
	"     (box (label 1) (pname S43) (atts (color lightPurple) (onclick \"clicked on S43\n\"))) " +
	"     (box (label 2) (pname RBD) (atts (color  C:RGB:0:0:255:200) (onclick \"clicked on RBD\n\"))) " +
	"     (box (label 3) (pname C1) (atts (color C:RGB:0:0:255:150) (onclick \"clicked on C1\n\"))) " +
	"     (box (label 4) (pname S259) (atts (color C:RGB:0:0:255:100) (onclick \"clicked on S259\n\") (modifiers (((box (label 4-1) (pname P) (atts (color purple) (onclick \"clicked on S259's modifier\n\")))  top)))))" +
	"     (box (label 5) (pname S338) (atts (color purple) (onclick \"clicked on S338\n\")))" +
	"     (box (label 6) (pname Y341) (atts (color purple) (onclick \"clicked on Y341\n\")))" +
	"     (box (label 7) (pname PABM) (atts (color lightBlue) (onclick \"clicked on PABM\n\")))" +
	"     (box (label 8) (pname S612) (atts (color purple) (onclick \"clicked on S621\n\") (modifiers (((box (label 8-1) (pname P) (atts (color purple) (onclick \"clicked on S612's modifier\n\")))  top)))))" +
        "   ) " +
	"   (bindings  " + 
               "((box (label 4b) (pname \"SBD\") (atts (color green) (onclick \"clicked on SBD[14-3-3]\n\"))) 4 top)" + 
               "((box (label 8b) (pname \"SBD\") (atts (color green) (onclick \"clicked on SBD[14-3-3]\n\"))) 8 top))" +
	"   (connections (4b 8b) (4 4b) (8 8b))" +
        "   (atts (x 10) (y 330))" +
       "  )";



}
/*
(box
  (label <string>)
  (pname <string>)
  (atts (color <color>)
        (onclick <msg>)
        (modifiers ((<box1> <pos>) ...))    <pos> is top or bottom
  )
)
*/

class PfdBox extends EzdGlyph  implements GraphActThing, Displayable  {
    private       String    label;
    private       String    pname;
    private       Color     color;
    private       String    onClick;
    private       int       height = 20;
    private       int       width  = 50;
    private       int       x;
    private       int       y;
    private       CJFrame   frame;
    private final ArrayList modifiers = new ArrayList();

    
    //Displayable interface
    public void addToDrawing(EzdDrawing drawing){
	drawing.add(this);
	ListIterator iter = modifiers.listIterator();
	while(iter.hasNext()){
	    EzdGlyph glyph = (EzdGlyph)iter.next();
	    drawing.add(glyph);
	}
    }

    public Rectangle getBounds(){
	Rectangle rect = new Rectangle(x, y, width, height);
	ListIterator iter = modifiers.listIterator();
	while(iter.hasNext()){
	    Displayable glyph = (Displayable)iter.next();
	    rect.add(glyph.getBounds());
	}
	return rect;
    }

    //GraphActThing Interface
    public String getGraphActLabel(){ return label; }
    public void   setGraphActLabel(String label){ this.label = label; }

    void setX(int x){ 
	this.x = x; 
	ListIterator iter = modifiers.listIterator();
	while(iter.hasNext()){
	    PfdBoxModifier modifier = (PfdBoxModifier)iter.next();
	    modifier.setX(x + width/4);
	}
    }
    void setY(int y){ 
	this.y = y; 
	ListIterator iter = modifiers.listIterator();
	while(iter.hasNext()){
	    PfdBoxModifier modifier = (PfdBoxModifier)iter.next();
	    if(modifier.getPosition() == Pfd.TOP){
		modifier.setY(y - height/2);
	    } else if(modifier.getPosition() == Pfd.BOTTOM){ 
		modifier.setY(y + height);
	    }
	}
    }
    int getX(){ return x; }
    int getY(){ return y; }

    


    ArrayList getModifiers(){ return modifiers; }
    
    int getWidth(){ return width; }
    void setWidth(int width){ this.width = width; }
    int getHeight(){ return height; }
    void setHeight(int height){ this.height = height; }

    public void mousePressed(EzdView v, MouseEvent me, int x, int y) {
        getDrawing().insertAt(this, EzdDrawing.TOP);
	ActorMsg.sendActorMsg(IO.out, onClick);
    }
    

    PfdBox(Sexp list, CJFrame frame){
	this.frame = frame;
	try{
	    parse(this, list);
	}catch(ParseError pe){ IO.err.println(pe); }
    }

    public void parse(PfdBox self, Sexp list) throws ParseError {
	if(list.length() != 4) 
	    throw new ParseError("PfdBox: not right length");
 	if(!(list.get(0) instanceof String))
	    throw new ParseError("PfdBox: didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("box"))
	    throw new ParseError("PfdBox: didn't start with \"box\"");
	if(!(list.get(1) instanceof Sexp))
	    throw new ParseError("PfdBox: label slot not a Sexp");
	Parser.parseLabel(self, (Sexp)list.get(1));
	if(!(list.get(2) instanceof Sexp))
	    throw new ParseError("PfdBox: pname slot not a Sexp");
	parsePname(self, (Sexp)list.get(2));
	if(!(list.get(3) instanceof Sexp))
	    throw new ParseError("Pfd:Box atts slot not a Sexp");
	parseAtts(self, (Sexp)list.get(3));
   }
   
    private void parsePname(PfdBox self, Object obj) throws ParseError {
	if(!(obj instanceof Sexp))throw new ParseError("Pfd:  pname not an Sexp");
	Sexp list = (Sexp)obj;
	if(list.length() != 2)
	    throw new ParseError("PfdBox: pname slot not right length");
   	if(!(list.get(0) instanceof String))
	    throw new ParseError("PfdBox: pname slot didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("pname"))
	    throw new ParseError("PfdBox: pname slot didn't start with \"pname\"");
   	if(!(list.get(1) instanceof String))
	    throw new ParseError("PfdBox: pname value not a String");
	self.pname = (String)list.get(1);
    }
    

    private void parseAtts(PfdBox self, Object obj) throws ParseError {
	if(!(obj instanceof Sexp))throw new ParseError("PfdBox:  atts slot not a Sexp");
	Sexp list = (Sexp)obj;
	if(list.length() == 0)
	    throw new ParseError("PfdBox: atts slot not right length");
	if(!(list.get(0) instanceof String))
	    throw new ParseError("PfdBox: atts slot didn't start with a String");
	if(!((String)list.get(0)).equalsIgnoreCase("atts"))
	    throw new ParseError("PfdBox: atts slot didn't start with \"atts\"");
	for(int i = 1; i < list.length(); i++){
	    if(!(list.get(i) instanceof Sexp))
		throw new ParseError("PfdBox: atts element " + i + " not a Sexp");
	    Sexp element = (Sexp)list.get(i);
	    if(element.length() != 2)
		throw new ParseError("PfdBox: atts element " + i + " not right length");
	    if(!(element.get(0) instanceof String))
		throw new ParseError("PfdBox: atts element " + i + " didn't start with a String");
	    if(((String)element.get(0)).equalsIgnoreCase("color")){
		self.color = Colour.string2Color((String)element.get(1));
	    } else if(((String)element.get(0)).equalsIgnoreCase("width")){
		self.width = Integer.parseInt((String)element.get(1));
	    } else if(((String)element.get(0)).equalsIgnoreCase("height")){
		self.height = Integer.parseInt((String)element.get(1));
	    } else if(((String)element.get(0)).equalsIgnoreCase("x")){
		self.x = Integer.parseInt((String)element.get(1));
		IO.err.println("Set x of " + pname + " to " + self.x);
	    } else if(((String)element.get(0)).equalsIgnoreCase("y")){
		self.y = Integer.parseInt((String)element.get(1));
		IO.err.println("Set y of " + pname + " to " + self.y);
	    } else if(((String)element.get(0)).equalsIgnoreCase("onclick")){
		self.onClick = (String)element.get(1);
	    } else if(((String)element.get(0)).equalsIgnoreCase("modifiers")){
		parseModifiers(self, element.get(1));
	    } else continue;
	}
    }
    /*
    (modifiers ((<box1> <pos>) ...))    <pos> is top or bottom
    */
    private void parseModifiers(PfdBox self, Object obj) throws ParseError {
	if(!(obj instanceof Sexp)) throw new ParseError("Pfd:  modifiers not an Sexp");
	Sexp list = (Sexp)obj;
	for(int i = 0; i < list.length(); i++){
	    if(!(list.get(i) instanceof Sexp))
		throw new ParseError("PfdBox: modifier element " + i + " not a Sexp");
	    Sexp element = (Sexp)list.get(i);
	    if(element.length() != 2)
		throw new ParseError("PfdBox: modifier element " + i + " not right length");
	    if(!(element.get(0) instanceof Sexp))
		throw new ParseError("PfdBox: modifier element " + i + "'s car not a Sexp");
	    if(!(element.get(1) instanceof String))
		throw new ParseError("PfdBox: modifier element " + i + "'s cdr not a String");
	    PfdBoxModifier modifier = new PfdBoxModifier((Sexp)element.get(0), this.frame);
	    modifier.setPosition((String)element.get(1));
	    modifiers.add(modifier);
	}
    }
    
    public Point getCenter(){
	return new Point(this.x + this.width/2, this.y + this.height/2);
    }

    public void draw(int style){
        if(style == Pfd.SPINE){
	    FontMetrics fm = frame.getFontMetrics(Pfd.textLarge);
            drawAs(new EzdFillRectangle( x, y, width, height, Color.white),
		   new EzdFillRectangle( x, y, width, height, color ),
                   new EzdRectangle( x, y, width, height, Color.black ),
		   new EzdString(pname, x, y, width, height, EzdString.CENTER, Color.black,  Pfd.textLarge, fm));
        } else if(style == Pfd.MODIFIER){
	    FontMetrics fm = frame.getFontMetrics(Pfd.textSmall);
            drawAs( new EzdFillRectangle( x, y, width/2, height/2, Color.white ),
		    new EzdFillRectangle( x, y, width/2, height/2, color ),
		    new EzdRectangle( x, y, width/2, height/2, Color.black ),
		    new EzdString(pname, x, y, width/2, height/2, EzdString.CENTER, Color.black, Pfd.textMedium, fm));
        } else if(style == Pfd.BINDING){
	    FontMetrics fm = frame.getFontMetrics(Pfd.textSmall);
            drawAs( new EzdFillRectangle( x, y, width, height, Color.white ),
		    new EzdFillRectangle( x, y, width, height, color ),
		    new EzdRectangle( x, y, width, height, Color.black ),
		    new EzdString(pname, x, y, width, height, EzdString.CENTER, Color.black, Pfd.textMedium, fm));
	}
	ListIterator iter = modifiers.listIterator();
	while(iter.hasNext()){
	    PfdBoxModifier modifier = (PfdBoxModifier)iter.next();
	    modifier.draw(Pfd.MODIFIER);
	}
    }
}

class PfdBoxModifier extends PfdBox {
    private int position;

    public int  getPosition(){ return position; }
    public void setPosition(int position){ this.position = position;}
    public void setPosition(String string){
	if(string == null) return;
	if(string.equalsIgnoreCase("top")){
	    setPosition(Pfd.TOP);
	} else if(string.equalsIgnoreCase("bottom")){
	    setPosition(Pfd.BOTTOM);
	}
    }

    PfdBoxModifier(Sexp list, CJFrame frame){
	super(list, frame);
    }
}

class PfdBoxBinding extends PfdBox {
    private int    position;
    private PfdBox site;

    public PfdBox getSite(){ return site; }
    public int  getPosition(){ return position; }
    public void setPosition(int position){ this.position = position;}
    public void setPosition(String string){
	if(string == null) return;
	if(string.equalsIgnoreCase("top")){
	    setPosition(Pfd.TOP);
	} else if(string.equalsIgnoreCase("bottom")){
	    setPosition(Pfd.BOTTOM);
	}
    }

    PfdBoxBinding(PfdBox site, Sexp list, CJFrame frame){
	super(list, frame);
	this.site = site;
	this.setHeight((2 * site.getHeight())/3);
    }
}

class PfdConnection extends EzdGlyph implements Displayable {
    private PfdBox    beg;
    private PfdBox    end;

    //Displayable interface
    public void addToDrawing(EzdDrawing drawing){
	drawing.add(this);
    }

    public Rectangle getBounds(){
	Rectangle box = new Rectangle();
	box.add(beg.getCenter());
	box.add(end.getCenter());
	return box;
    }

    public PfdConnection(PfdBox beg, PfdBox end){
	this.beg = beg;
	this.end = end;
    }

    void draw(){
	Point p0 = beg.getCenter();
	Point p1 = end.getCenter();
	drawAs(new EzdLine(p0.x, p0.y, p1.x, p1.y, Color.black));
    }
	   
}

class PfdLabel extends EzdGlyph implements Displayable {
    protected String  label;
    protected int     x;
    protected int     y;
    protected int     height = 20;
    protected int     width  = 150;
    protected CJFrame frame;


    //Displayable interface
    public void addToDrawing(EzdDrawing drawing){
	drawing.add(this);
    }
    public Rectangle getBounds(){
	return new Rectangle(x, y, width, height);
    }

    PfdLabel(String label, CJFrame frame){
	this.label = label;
	this.frame = frame;
    }

    void draw(){
	FontMetrics fm = frame.getFontMetrics(Pfd.textLarge);
	drawAs(new EzdString(label, x, y, width, height, EzdString.CENTER, Color.black,  Pfd.textLarge, fm));
    }
}
