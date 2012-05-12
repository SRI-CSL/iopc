package dv;

import g2d.swing.IOPConstants;
import g2d.jlambda.*;
import g2d.Main;

import java.net.Socket;
import java.io.IOException;
import java.io.InputStream;

/**
 * Entry to running the dumb viewer as a Jave Web Start application.
 */
public class DumbViewer {

    public DumbViewer() {
        // load "pla.lsp"
        StringBuffer sb = new StringBuffer();
        InputStream is = getClass().getResourceAsStream("pla.lsp");
        try {
            int c, p = -1;
            boolean inComment = false;
            while ((c = is.read()) != -1) {
                if (!inComment){
                    if ((c == IOPConstants.COMMENT)
                        && ((p == -1) || Character.isWhitespace((char)p))) {
                        inComment = true;
                        sb.append(" \n");
                    } else {
                        sb.append((char) c);
                    }
                } else {
                    if (c == '\n') inComment = false;
                }
                p = c;
            }
        } catch (Exception e) {
            System.err.println(e);
        }
        // evaluate "pla.lsp"
        try {
            Evaluate.evaluate(sb.toString());
        } catch (ParseError parseError) {
            System.err.println(parseError);
        } catch (SyntaxError syntaxError) {
            System.err.println(syntaxError);
        }
        
        // call g2d.Main(String[] args)
        String[] args = {"dumbviewer"};
        Main.main(args);
    
    }
    
    public static void main(String[] args){
        Main.setBinaryDirectory(".");
        // parse port number and create object
        new DumbViewer();
    }
}
