(let 
    (
     (x0 (int 210))
     (y0 (int 150))	 
     (x1 (int 50))
     (y1 (int 300))	 
     (trans0 (let ((temp (object ("java.awt.geom.AffineTransform"))))
	      (seq (invoke temp "translate" x0 y0) temp)))
     (trans1 (let ((temp (object ("java.awt.geom.AffineTransform"))))
	      (seq (invoke temp "translate" x1 y1) temp)))
     ;; g0.png is 177 × 105 pixels
     (img0 (object ("g2d.glyph.ImageGlyph" "g0.png")))
     ;; g1.png is 138 × 84 pixels
     (img1 (object ("g2d.glyph.ImageGlyph" "g1.png")))
     
     ;; some odd 1-off error?
     (rect0 (object ("java.awt.geom.Rectangle2D$Double" x0 y0 (int 176) (int 104))));
     (glyph0 (object ("g2d.glyph.Glyph" rect0 java.awt.Color.black java.awt.Color.black)))

     ;; some odd 1-off error?
     (rect1 (object ("java.awt.geom.Rectangle2D$Double" x1 y1 (int 137) (int 83))));
     (glyph1 (object ("g2d.glyph.Glyph" rect1 java.awt.Color.black java.awt.Color.black)))

     (view (object ("g2d.swing.IOPView")))
     )
  (seq
   (invoke view "add" glyph0) 
   (invoke view "add" img0 trans0) 

   (invoke view "add" glyph1) 
   (invoke view "add" img1 trans1) 
   (invoke  (object ("g2d.swing.IOPFrame" "Image Example" view)) "setVisible" (boolean true))
   )
  )


  
/*

RectangularShape rshape = new Ellipse2D.Double(x - w, y - h, 2*w, 2*h);
Glyph glyph = new Glyph(rshape, java.awt.Color.black, fill); 


RectangularShape baseShape = new RoundRectangle2D.Double(x0, graphHeight - y1, w, h, deltaX, deltaY);
this.base = new Glyph(baseShape, java.awt.Color.black, this.fill); 

RectangularShape baseShape = new Rectangle2D.Double(x0, graphHeight - y1, w, h);
this.base = new Glyph(baseShape, java.awt.Color.black, this.fill); 



*/
