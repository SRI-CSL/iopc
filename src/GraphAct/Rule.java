package GraphAct;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.TextArea;
import java.awt.Font;
import java.awt.event.KeyEvent;
import java.awt.event.KeyAdapter;
import java.util.ArrayList;
import java.util.ListIterator;

public class Rule implements GraphActWindowThing {
    private static final boolean DEBUG  = false;
    private static final int padding = getOSPadding();

    private String label;
    private final CJFrame frame = new CJFrame();


    public String getGraphActLabel(){ return label; }
    public void setGraphActLabel(String label){ this.label = label; }

    public Rule(String msg){
	int beg = msg.indexOf('"');
	int end = msg.lastIndexOf('"');
	if((beg < 0) || (end < 0) || (beg == end)) return;
	label = msg.substring(beg + 1, end);
	if(DEBUG){
	    IO.err.println("label = " + label);
	    IO.err.println(preferedSize(label));
	}
    }

    static int getOSPadding(){
	String os = System.getProperty("os.name");
	if(os.toLowerCase().equals("mac os x")) 
	    return 6;
	else 
	    return 2;
    }

    Dimension preferedSize(String label){
	if(label == null) label = "\n(null)\n";
	int x = 0, y = 0, c = 0, len = label.length();
	for(int i = 0; i < len; i++){
	    if(label.charAt(i) != '\n'){
		c++;
	    } else {
		y++;
		if(c > x) x = c;
		c = 0;
	    }
	}
	x = (x < 20) ? 20 : x;
	y = (y < 4) ? 4 : y;
	return new Dimension(x, y + padding);
    }

    public void dispose(){
	frame.dispose();
    }

    public boolean isVisible(){
	return frame.isVisible();
    }

    public void display(){
	new Thread(){
	    public void run(){
		String text;
		if(label == null) label = "Unknown";
		int index = label.indexOf(':');
		String title = (index < 0) ?  label : label.substring(0, index);
		text = "\n" + label + "\n";
		final Dimension d = Rule.this.preferedSize(label);
		frame.setTitle(title);
		final TextArea ta = new TextArea(text, d.height, d.width);
		ta.setBackground(Color.white);
		//		ta.setFont(GUI.Constants.iopFont(Font.PLAIN, 12));
		ta.setEditable(false);
		frame.add(ta);
		frame.pack();
		ta.addKeyListener(new KeyAdapter(){
			public void keyPressed(KeyEvent ke){
			    if(DEBUG)IO.err.println(ke);
			    if(ke.getKeyCode() == KeyEvent.VK_Q)frame.dispose();
			}
		    });
		frame.setVisible(true);
	    }
	}.start();
    }
}
