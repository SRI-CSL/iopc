 (graph
 (label "raf Graph")
 (nodes
  (node 0 ((shape circle) 
	   (label "FN[out]") 
	   (level 7)
	   (color white) 
	   (border black)))
  (node 1 ())
  (node 2 ())              
  (node 3 ((shape square) 
	   (label 526) 
	   (color white) 
	   (border black)
	   (onclick "maude\ngraphics\nshow rule 526")))
  )
 (edges 
  (edge 0 3 ((color green) (label "i")))
  (edge 1 3 ((color blue) (label "i")))
  (edge 3 2 ((color gray) (label "o")))
  )
 
 [optional menubar]

 )
 

(string "this is a string")

(extend 
 (focus 7)          //id of node clicked on.
 (label "my name")  //graph to extend.
 (nodes ...)
 (edges ...)
)

(delete 
 (label "my name")  //graph to delete
 (nodes ...)
 (edges ...)
)

(menubar
 (menu (label "file")  
       (menuitem 
	(label "quit") 
	(action "maude\n\viewer\nqids-for-maude")
	(docstring "Q") 
	(shortcut "keycode") 
	)
       (menuitem ...)
       (menuitem ...) 
       )
 (menu  ... )
 (menu  ... )
 )


----------------------------------------------------------------------------------------------

(grid 
 (label name)
 (rows r)
 (columns c)
 (contents (obstacle id r c ) .... (rover id rR cR rDir aDir))
 )

(gridCmd name (id methodName arg1 arg2 ... argN)  ...   (id methodName arg1 arg2 ... argN))


(move)

(rotate dir)

(swing dir)  //relative to rover


dir in { N E S W }

----------------------------------------------------------------------------------------------

(canvas 
 (label name)
 (sprite ...)
 (sprite ...)
 (sprite ...)
 (sprite ...)
)




(sprite 
  (label  <name>)
  (images <directory>)
  (width  <width>)
  (height <height>)
  (xpos   <xpos>)
  (ypos   <ypos>)
  (view   <imagefilename>) %no format extension
)

(command name cmd0 cmd1 ...)


(sprite-name set <att> <val>)
(sprite-name sleep <n millseconds>)
(sprite-name repaint)
(sprite-name get <cust> <att1>  ... <attN>)  
   ===> "maude\ngraphics\n<cust> <container> sprite-name getReply <att1> <val1> ... <attK> <valK>"


Neko
 " size = " + size + 
				   
RIGHT1
RIGHT2
STOP
YAWN
SCRATCH1
SCRATCH2
SLEEP1
SLEEP2
AWAKE


----------------------------------------------------------------------------------------------


(pfd
  (label <string>)
  (pname <string>)
  (spine (<box1> ... <boxk>))
  (bindings (<box1> <label> <pos>) ... )     *** pos is top means above spine
  (connections (<label1a> <label1b>) ... )   *** bottom means below spine
  (atts ...)
 )

(box
  (label <string>)
  (pname <string>)
  (atts (color <color>)
        (onclick <msg>)
        (modifiers ((<box1> <pos>) ...))    *** <pos> is top or bottom
  )
)


(pfdcanvas
  (label <string>)
  (pname <string>)
  (pfdlist (pfd ...) ... (pfd ...))
  (pfdarrowlist )
  (atts (background <color>) (width <num>) (height <num>) ... )
  [optional menu bar]
)


(pfdextend
   (label <string>)
   (pfdlist (pfd ...) ... (pfd ...))
   (pfdarrowlist )
)

(pfddelete
   (label <string>)
   (pfdlabellist <label> ... <label>)
   (pfdarrowlabellist )
)

(pfdarrow  
   (label <string>)
   (pname <string>)
   (source <label>)
   (target <label>)
   (atts (color  <color>)
	 (width  <int>)  % pixel
	 (length <int>)  % percentage
   )
)
