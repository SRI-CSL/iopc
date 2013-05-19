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
     (img0 (object ("g2d.glyph.ImageGlyph" "g0.png")))
     (img1 (object ("g2d.glyph.ImageGlyph" "g1.png")))
     (view (object ("g2d.swing.IOPView")))
     )
  (seq
   (invoke view "add" img0 trans0) 
   (invoke view "add" img1 trans1) 
   (invoke  (object ("g2d.swing.IOPFrame" "Image Example" view)) "setVisible" (boolean true))
   )
  )


  
