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
        InputStream is = getClass().getResourceAsStream("g2dlib.lsp");
        // load "pla.lsp"
        String lispfile = is2sb_lsp(is);
        
        //System.err.println(lispfile);

        //g2d.util.IO.string2File(lispfile, "parsed.lsp");

        eval(lispfile);

        is = getClass().getResourceAsStream("graph2.lsp");
        
        lispfile = is2sb_lsp(is);


        is = getClass().getResourceAsStream("graph2.xdot");

        String dotfile = is2sb_txt(is);

        //System.err.println(dotfile);
        
        eval(lispfile);
        

    }

    void eval(String code){
        try {
            Evaluate.evaluate(code);
        } catch (ParseError parseError) {
            System.err.println(parseError);
        } catch (SyntaxError syntaxError) {
            System.err.println(syntaxError);
        }
    }

    /*** WON'T JLAMBDA DO THIS FOR ME???? ****/
    
    String is2sb_lsp(InputStream is){
        StringBuffer sb = new StringBuffer();
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
        return sb == null ? "" : sb.toString();
    }
    
    String is2sb_txt(InputStream is){
        StringBuffer sb = new StringBuffer();
        try {
            int c;
            while ((c = is.read()) != -1) {
                sb.append((char) c);
            }
        } catch (Exception e) {
            System.err.println(e);
        }
        return sb == null ? "" : sb.toString();
    }
    
    public static void main(String[] args){
        Main.setBinaryDirectory(".");
        // parse port number and create object
        new DumbViewer();
    }
}
