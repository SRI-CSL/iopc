/*
 * See copyright notice in this directory!
 */

package Ezd;

import java.awt.Image;
import java.awt.Dimension;

import java.awt.event.MouseEvent;

import java.util.ArrayList;
import java.util.ListIterator;


public class Ezd {

    static String mutex = "Mutual Exclusion Lock";

    static EzdView mouseView = null;       // Mouse in this view,
    static int mouseX = 0;                 // at this position,
    static int mouseY = 0;
    static int modifiers = 0;
    static boolean mouseDamaged = false;   // Mouse is in a damaged area.
    static boolean eventHandler = false;   // Inside an event handler
    static ArrayList deferredViews = null;    // Views that have repaints defered
                                           // because they were damaged inside an
                                           // event handler.
    static Image offImage = null;          // Off screen Image buffer
    static Dimension offDimension = null;  // Off screen Image buffer size

    Ezd() {   }

    public static String version() {   return "September 2003";   }

    static void setMouse(EzdView c, MouseEvent me){
        mouseView = c;
        mouseX = me.getX();
        mouseY = me.getY();
        modifiers = me.getModifiers();
    }

    static void enterHandler() {
        eventHandler = true;
        deferredViews = new ArrayList();
    }

    static void exitHandler() {
        EzdView c;
	ListIterator iter = deferredViews.listIterator();
	while(iter.hasNext()){
	    c = (EzdView)iter.next();
            c.repaint(c.deferredDamage.x, c.deferredDamage.y, c.deferredDamage.width, c.deferredDamage.height);
            c.deferredDamage = null;
	}
        deferredViews = null;
        eventHandler = false;
    }

    static boolean insideHandler() {
        return eventHandler;
    }

    static void deferRepaint( EzdView c ) {
        if (deferredViews.contains( c )) return;
        deferredViews.add( c );
    }

}
