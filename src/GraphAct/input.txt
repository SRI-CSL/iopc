(user 
(pfdcanvas
 (label testCanvas)
 (pname "PDFCanvas 01: Activated PKCz")
 (pfdlist 
  (pfd 
   (label  "PKCz-act") 
   (pname  "PKCz activated") 
   (spine 
    (box (label 1) (pname S43) (atts (color lightPurple) (onclick "clicked on S43\n"))) 
    (box (label 2) (pname RBD) (atts (color blue) (onclick "clicked on RBD\n"))) 
    (box (label 3) (pname C1) (atts (color blue) (onclick "clicked on C1\n"))) 
    (box (label 4) (pname S259) (atts (color purple) (onclick "clicked on S259\n") (modifiers (((box (label 4.1) (pname P) (atts (color purple) (onclick "clicked on S259's modifier\n")))  bottom)))))
    (box (label 5) (pname S338) (atts (color purple) (onclick "clicked on S338\n")))
    (box (label 6) (pname Y341) (atts (color purple) (onclick "clicked on Y341\n")))
    (box (label 7) (pname PABM) (atts (color lightBlue) (onclick "clicked on PABM\n")))
    (box (label 8) (pname S612) (atts (color purple) (onclick "clicked on S621\n") (modifiers (((box (label 8.1) (pname P) (atts (color purple) (onclick "clicked on S612's modifier\n")))  bottom)))))
    ) 
   (bindings	((box (label 4b) (pname "SBD") (atts (color green) (onclick "clicked on SBD[14-3-3]\n"))) 4 bottom) ((box (label 8b) (pname "SBD") (atts (color green) (onclick "clicked on SBD[14-3-3]\n"))) 8 bottom))
   (connections (4b 8b) (4 4b) (8 8b))
   (atts (x 10) (y 240))
   )
  )
 (atts (background white) (width 600) (height 400))
 (menubar
  (menu (label file)  
	(menuitem 
	 (label quit) 
	 (action "maude\nviewer\nqids-for-maude\n")
	 (docstring Q) 
	 (shortcut Q) 
	 )
	)
  )  
 )
)
