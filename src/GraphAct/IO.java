package GraphAct;

import java.io.PrintStream;
import java.io.InputStream;

public class IO {
    protected static InputStream in  = System.in;
    protected static PrintStream out = System.out;
    protected static PrintStream err = System.err;


    public static void setIO(InputStream inv, PrintStream outv, PrintStream errv){
	in = inv;
	out = outv;
	err = errv; 
    }

}
