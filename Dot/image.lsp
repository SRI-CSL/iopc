//need to add a mouse click to check inside() etc ...
(sinvoke "g2d.jlambda.Debugger" "toggleVerbosity")
(sinvoke "g2d.jlambda.Debugger" "setStackDepth" (int 6))

(let 
    (
     (x0 (int 210))
     (y0 (int 150))	 

     (x1 (int 50))
     (y1 (int 300))	 

     (x2 (int 300))
     (y2 (int 500))	 

     ;; g0.png is 177 × 105 pixels
     (img0 (object ("g2d.glyph.ImageGlyph" "g0.png" x0 y0)))

     ;; g1.png is 138 × 84 pixels
     (img1 (object ("g2d.glyph.ImageGlyph" "g1.png" x1 y1)))
     
     ;; some odd 1-off error?
     (rect0 (object ("java.awt.geom.Rectangle2D$Double" x0 y0 (int 176) (int 104))));
     (glyph0 (object ("g2d.glyph.Glyph" rect0 java.awt.Color.black java.awt.Color.black)))

     ;; some odd 1-off error?
     (rect1 (object ("java.awt.geom.Rectangle2D$Double" x1 y1 (int 137) (int 83))));
     (glyph1 (object ("g2d.glyph.Glyph" rect1 java.awt.Color.black java.awt.Color.black)))

     (img2 (object ("g2d.glyph.ImageGlyph" "g1.png" x2 y2 (int 69) (int 41))))
     (rect2 (object ("java.awt.geom.Rectangle2D$Double" x2 y2 (int 69) (int 41))));
     (glyph2 (object ("g2d.glyph.Glyph" rect2 java.awt.Color.black java.awt.Color.black)))

     (view (object ("g2d.swing.IOPView" (boolean true) (boolean true))))
     )
  (seq
   (invoke view "add" glyph0) 
   (invoke view "add" img0)

   (invoke view "add" glyph1) 
   (invoke view "add" img1) 

   (invoke view "add" glyph2) 
   (invoke view "add" img2) 

   (invoke  (object ("g2d.swing.IOPFrame" "Image Example" view)) "setVisible" (boolean true))
   )
  )


  
