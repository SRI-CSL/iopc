/*
 * See copyright notice in this directory!
 */

package Ezd;

import java.awt.Graphics;
import java.awt.Rectangle;

public abstract class EzdGraphic {

    abstract void move( int dx, int dy );

    abstract void paint( Graphics g, EzdMap m );

    abstract Rectangle boundingRect( EzdMap m );

    abstract boolean intersects( EzdMap m, Rectangle r );
}
