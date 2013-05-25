(seq 
 (sinvoke "g2d.jlambda.Debugger" "toggleVerbosity")
 ;(supdate "g2d.graph.IOPNode" "BORDER_COUNT_DEFAULT" (int 2))
 ;(supdate "g2d.graph.IOPNode" "BORDER_GAP" (int 4))
 (define display 
   (lambda (dotfile)
     (let (
           (view (object ("g2d.swing.IOPView" (boolean true) (boolean true))))
           (frame (object ("g2d.swing.IOPFrame" "A Graph (layout by dot)" view)))
           (graph (object ("g2d.graph.IOPGraph")))
           (file (object ("java.io.File" dotfile)))


           (clicked (lambda (self event)
                      (let (
                            (point (object ("java.awt.geom.Point2D$Double" 
                                            (invoke event "getX")
                                            (invoke event "getY"))))
                            (node (invoke graph "getNode" point))
                            (gl (if (isobject node) (lookup node "glyphList") "not a node"))
                            )
                        (seq
                         (if (isobject node)
                             (let (
                                   (label (invoke node "getName")))
                               (seq
                                (invoke node "highlight")
                                (invoke view "repaint")
                                (invoke java.lang.System.err "println" gl))))))))
           )
       (seq
        (invoke graph "setGraph" file)
        ;    (supdate "g2d.graph.Dot" "dotDebug" (boolean true))
        (invoke view "add" graph)
        (invoke frame "setVisible" (boolean true))
        (invoke view "setMouseAction" java.awt.event.MouseEvent.MOUSE_CLICKED clicked)
        (invoke view "repaint")))
     )
   )
 ;;turn on verbose toStrings for clicking and debugging
 (supdate "g2d.glyph.GlyphList" "debug" (boolean true))
 (supdate "g2d.glyph.Glyph" "debug" (boolean true))
 (supdate "g2d.glyph.TextGlyph" "debug" (boolean true))
 (supdate "g2d.glyph.ImageGlyph" "debug" (boolean true))
 (apply display "stable.dot")
)