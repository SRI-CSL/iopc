package GraphAct;

import java.awt.Color;

import java.util.StringTokenizer;

import javax.swing.JColorChooser;

public class Colour {
    
    public static Color purple = new Color(156, 0, 156);
    public static Color lightPurple = new Color(204, 153, 255);
    public static Color lightBlue = new Color(153, 153, 255);
    public static Color lightCyan = new Color(153, 255, 255);

    /*
      You can also specify the precise RGB by using a string of the form:

      C:RGB:<num>:<num>:<num>[:<num>]
    
      Where each <num> is an integer in the range 0 to 255.
      The optional 4th number is the alpha, or opaqueness, 255 is completely
      opaque, and C:RGB:N1:N2:N3:255 is the same as C:RGB:N1:N2:N3 

    */

    public static boolean isRGBString(String s){
	return s.toLowerCase().startsWith("c:rgb:");
    }

    public static Color RGBColor(String s){
	Color retval = Color.black;
	int red, green, blue, alpha = -1;
	StringTokenizer tokenizer = new StringTokenizer(s, ":");
	if(!tokenizer.hasMoreTokens()) return retval;
	tokenizer.nextToken();
	if(!tokenizer.hasMoreTokens()) return retval;
	tokenizer.nextToken();
	if(!tokenizer.hasMoreTokens()) return retval;
	red = Integer.parseInt(tokenizer.nextToken());
	if(!tokenizer.hasMoreTokens()) return retval;
	green = Integer.parseInt(tokenizer.nextToken());
	if(!tokenizer.hasMoreTokens()) return retval;
	blue = Integer.parseInt(tokenizer.nextToken());
	if(!tokenizer.hasMoreTokens()) return retval;
	alpha = Integer.parseInt(tokenizer.nextToken());
	if(alpha < 0)
	    retval = new Color(red, green, blue);
	else 
	    retval = new Color(red, green, blue, alpha);
	return retval;
    }

    public static String RGBString(Color c){
	if(c == null) return "black";
	if(c.getAlpha() == 255)
	    return "C:RGB:" + c.getRed() + ":" + c.getGreen() + 
		":" + c.getBlue();
	else 
	    return "C:RGB:" + c.getRed() + ":" + c.getGreen() + 
		":" + c.getBlue() + ":" + c.getAlpha();
    }

    public static Color string2Color(String s){
	if(s == null) return Color.black;
	if(isRGBString(s)) return RGBColor(s);
	if(s.equalsIgnoreCase("black")) return Color.black;
	if(s.equalsIgnoreCase("blue")) return Color.blue;
	if(s.equalsIgnoreCase("cyan")) return Color.cyan;
	if(s.equalsIgnoreCase("darkGray")) return Color.darkGray;
	if(s.equalsIgnoreCase("gray")) return Color.gray;
	if(s.equalsIgnoreCase("green")) return Color.green;
	if(s.equalsIgnoreCase("lightGray")) return Color.lightGray;
	if(s.equalsIgnoreCase("magenta")) return Color.magenta;
	if(s.equalsIgnoreCase("orange")) return Color.orange;
	if(s.equalsIgnoreCase("pink")) return Color.pink;
	if(s.equalsIgnoreCase("red")) return Color.red;
	if(s.equalsIgnoreCase("white")) return Color.white;
	if(s.equalsIgnoreCase("yellow")) return Color.yellow;
	if(s.equalsIgnoreCase("purple")) return purple;
	if(s.equalsIgnoreCase("lightBlue")) return lightBlue;
	if(s.equalsIgnoreCase("lightPurple")) return lightPurple;
	if(s.equalsIgnoreCase("lightCyan")) return lightCyan;
	return Color.black;
    }

    public static String color2String(Color c){
	if(c == null) return "black";
	if(c.equals(Color.black)) return "black";
	if(c.equals(Color.blue)) return "blue";
	if(c.equals(Color.cyan)) return "cyan";
	if(c.equals(Color.darkGray)) return "darkGray";
	if(c.equals(Color.gray)) return "gray";
	if(c.equals(Color.green)) return "green";
	if(c.equals(Color.lightGray)) return "lightGray";
	if(c.equals(Color.magenta)) return "magenta";
	if(c.equals(Color.orange)) return "orange";
	if(c.equals(Color.pink)) return "pink";
	if(c.equals(Color.red)) return "red";
	if(c.equals(Color.white)) return "white";
	if(c.equals(Color.yellow)) return "yellow";
	if(c.equals(purple)) return "purple";
	if(c.equals(lightBlue)) return "lightBlue";
	if(c.equals(lightPurple)) return "lightPurple";
	if(c.equals(lightCyan)) return "lightCyan";
	return RGBString(c);
    }

    public static void main(String[] args){
	CJFrame f = new CJFrame("CChooser");
	JColorChooser chooser = new JColorChooser();
	Color choice = chooser.showDialog(f, "CChooser", Color.white);
	f.dispose();
	IO.err.println(choice);
    }

}
