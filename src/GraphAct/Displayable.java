package GraphAct;

import java.awt.Rectangle;

import Ezd.EzdDrawing;

public interface Displayable {
    public void addToDrawing(EzdDrawing drawing);
    public Rectangle getBounds();
}
