(seq

;;; *********** DEBUG settings *********

; define verbosity
; (supdate "g2d.util.ActorMsg" "VERBOSE" (boolean true))
(sinvoke "g2d.jlambda.Debugger" "setVerbosity" (boolean true))

; if true prints lola retcode and path to GUI output
(define lolaDebug (boolean false))

;;; *********** colors *********

;lavendar
(define noneFillColor (object ("java.awt.Color" (int 210) (int 202) (int 255))))
;dklavendar
(define initFillColor (object ("java.awt.Color" (int 150) (int 150) (int 255))))
;ltgreen
(define goalFillColor (object ("java.awt.Color" (int 100) (int 255) (int 90))))
;red
(define ngoalFillColor (object ("java.awt.Color" (int 255) (int 0) (int 0))))
;orangeish
(define avoidFillColor (object ("java.awt.Color" (int 255) (int 136) (int 102))))
;lighter ltgreen
(define usesFillColor (object ("java.awt.Color" (int 180) (int 255) (int 160))))
;bluegreen
(define foundFillColor (object ("java.awt.Color" (int 0) (int 255) (int 255))))
;black
(define nodeBorderColor java.awt.Color.black)
;grey
;(define cxtBorderColor (object ("java.awt.Color" (int 204) (int 204) (int 204))))
(define cxtBorderColor java.awt.Color.gray)
;ltgrey
;(define cxtFillColor (object ("java.awt.Color" (int 204) (int 204) (int 255))))
(define cxtFillColor java.awt.Color.white)
;ltyellow
(define ruleFillColor (object ("java.awt.Color" (int 251) (int 255) (int 145))))
;darker dklavendar
(define bidirEdgeColor (object ("java.awt.Color" (int 50) (int 25) (int 255))))
;black
(define unidirEdgeColor java.awt.Color.black)

; tell Java about the colors
(supdate "pla.graph.PLAGraph" "noneFillColor" noneFillColor)
(supdate "pla.graph.PLAGraph" "initFillColor" initFillColor)
(supdate "pla.graph.PLAGraph" "goalFillColor" goalFillColor)
(supdate "pla.graph.PLAGraph" "ngoalFillColor" ngoalFillColor)
(supdate "pla.graph.PLAGraph" "avoidFillColor" avoidFillColor)
(supdate "pla.graph.PLAGraph" "usesFillColor" usesFillColor)
(supdate "pla.graph.PLAGraph" "foundFillColor" foundFillColor)
(supdate "pla.graph.PLAGraph" "nodeBorderColor" nodeBorderColor)
(supdate "pla.graph.PLAGraph" "cxtBorderColor" cxtBorderColor)
(supdate "pla.graph.PLAGraph" "cxtFillColor" cxtFillColor)
(supdate "pla.graph.PLAGraph" "ruleFillColor" ruleFillColor)
(supdate "pla.graph.PLAGraph" "bidirEdgeColor" bidirEdgeColor)
(supdate "pla.graph.PLAGraph" "unidirEdgeColor" unidirEdgeColor)

; bool is false if node in context not in net

(define getNodeBorderColor (bool)
  (if bool nodeBorderColor cxtBorderColor)
)

(define getOccFillColor (status bool)
   (if (= status "init") initFillColor
   (if (= status "goal") 
       (if bool goalFillColor ngoalFillColor)
   (if (= status "avoid") avoidFillColor
   (if bool noneFillColor cxtFillColor))))
)

(define getRuleFillColor (status bool)
  (if (= status "hide") avoidFillColor
  (if bool ruleFillColor cxtFillColor))
)

(define getEdgeColor (type bool)
  (if (not bool) cxtBorderColor
  (if (= type "bidir") bidirEdgeColor unidirEdgeColor))
)

;;; *********** interfaces *********

; from ii.maude -----------

; echos messages sent by g2d to GUI output window
(define sendMessage (to from msg)
  (sinvoke "g2d.util.ActorMsg" "send" to from msg)
) ;sendMessage

;(apply displayMessage2G %gname %title %message)
(define displayMessage2G (gname title msg)
  (seq
   (invoke (invoke (invoke (invoke (getAttr (fetch gname) "frame") 
				   "getGraphPanel") 
			   "getFrame") 
		   "getSEPanel") 
	   "displayText" title msg)
   ) ;seq
) ;displayMessage2G

;(apply displayMessage %title %message)
(define displayMessage (title msg)
  (seq
   ; hide any progress dialog!
   (sinvoke "pla.PLAFrame" "hideProgressDialog")
   ; show message in warning dialog:
   (sinvoke "javax.swing.JOptionPane" "showMessageDialog" 
	    (object null)
	    msg
	    title
	    javax.swing.JOptionPane.WARNING_MESSAGE)
   )
)

;(apply setDishes2F %fname %string-array-of-dishes)
(define setDishes2F (fname dishes)
  (let ((frame (invoke (fetch fname) "getFrame"))
	) 
    (invoke frame "displayDishes" dishes)
    )
) ;setDishes2F

;(apply lolaRequest %net %task %id %requestor)
;; CLT split in to local request and remote resquest server
(define lolaRequest (net task reqId requestor)
  (let ((res (apply doLolaReq net task reqId)))
     (apply sendMessage requestor "graphics2d"
	    (concat (aget res (int 0)) " " (aget res (int 1))))
	)
)

(define doLolaReq (net task reqId)
  (let ((lolaNetFile (invoke (sinvoke "java.io.File" "createTempFile" 
				      (concat "lola" reqId "_") ".net")
			     "getAbsolutePath")) 
	(lolaTaskFile (invoke (sinvoke "java.io.File" "createTempFile" 
				       (concat "lola" reqId "_") ".task")
			     "getAbsolutePath")) 
	(lolaPathFile (invoke (sinvoke "java.io.File" "createTempFile" 
				       (concat "lola" reqId "_") ".path")
			     "getAbsolutePath")) 
	(command (concat "lola " lolaNetFile 
			 " -a " lolaTaskFile
			 " -p " lolaPathFile))
	(res (mkarray java.lang.String (int 2)))
	)
    (seq
     (sinvoke "g2d.util.IO" "string2File" net lolaNetFile)
     (sinvoke "g2d.util.IO" "string2File" task lolaTaskFile)
; run lola       
; retcode == 0 => SATISFIED
; retcode == 1 => NOT-SAT
; retcode =/= 0 => UNDECIDED
     (try 
      (let ((lolaProc (invoke (sinvoke "java.lang.Runtime" "getRuntime")
			      "exec" command))
	    (dummy
	     (if lolaDebug
		 (invoke java.lang.System.err "println" "lolaProc started")))
	    (retcode (invoke lolaProc "waitFor"))
	    (resPath
	     (if (= retcode (int 0))
		 (sinvoke "g2d.util.IO" "file2String" lolaPathFile)
	       ""))
	    )
	(seq 
	 (if lolaDebug
	     (seq
	      (invoke java.lang.System.err "println" 
		      (concat "retcode: " retcode))
	      (invoke java.lang.System.err "println" 
		      (concat "resPath: " resPath))
	      ))  ; if
	 (aset res (int 0) (concat "" retcode))
	 (aset res (int 1) resPath)
	 )) ; seq;let
    (catch var 
      (seq (apply displayMessage "error" "run lola failed")
	     (aset res (int 0) (concat "" (- (int 1))))
	     (aset res (int 1) "")) 
	  ) ; catch
   ) ;try
   (if (not lolaDebug)
	 (seq
	  (sinvoke "g2d.util.IO" "deleteFile" lolaNetFile)
	  (sinvoke "g2d.util.IO" "deleteFile" lolaTaskFile)
	  (sinvoke "g2d.util.IO" "deleteFile" lolaPathFile)))
     res
     ))  ;seq;let
) ; doLolaReq


; from dg2g2d.maude -----------
; open new graph in new frame
(define PLgraph (id title subtitle issubnet makeGraph)
  (apply PLgraph2F 
	 (invoke (invoke (apply makeFrame (object ("pla.PLAMainFrame" ""))) 
			 "getID") "getUID")
	 id title subtitle issubnet makeGraph)
) ;PLgraph

; open new graph in frame given by "fname"
(define PLgraph2F (fname id title subtitle issubnet makeGraph)
  (let ((frame (invoke (fetch fname) "getFrame"))
	(graph (object ("pla.graph.PLAGraph")))
	) 
    (seq
     ; make graph 
     (invoke graph "setStrokeWidth" (float 1.0))
     (apply makeGraph graph)
     (invoke graph "setUID" id)
     ; do layout with dot by default
     (invoke graph "doLayout" (object null))
     ; update frame and set graph into frame
     (invoke frame "setTitle" title)
     (setAttr graph "frame" frame) ; remember frame of this graph
     (invoke frame "setGraph" graph)
     ; set categorization to default if top-level net
     (if (instanceof frame "pla.PLAMainFrame")
	 (invoke frame "setCategorization" (object null)))
     ; set subtitle if applicable
     (if (instanceof frame "pla.PLASubFrame")
	 (invoke frame "setSubtitle" subtitle))
     graph))
) ;PLgraph2F

; create a subframe of the graphs frame and send to requestor
(define newSubFrameRequest (gname requestor)
  (let ((frame (getAttr (fetch gname) "frame" (object null)))
       (subframe (invoke (invoke
                    (apply makeFrame (object ("pla.PLASubFrame" "" frame)))
          			 "getID") "getUID")))
   (apply sendMessage requestor "graphics2d" subframe)
  )
) ; newSubFrame


; add an occurrence node to given graph

(define maxLabLen (int 12))

(define addONode (graph nodearray lab clab nid status subnet)
  (let ((node (object ("pla.graph.PLANode"
		       (object ("pla.data.Occurrence" 
;; newcomplex
				(if (> (invoke lab "length") maxLabLen)
				    (invoke lab "substring" (int 0) maxLabLen) 
				    lab)
				(sinvoke "java.lang.Integer" "parseInt" nid)
				clab))
		       "ellipse"
		       (apply getNodeBorderColor (= subnet "true"))
		       (apply getOccFillColor status (= subnet "true")))))
	)
    (seq 
     (aset nodearray (sinvoke "java.lang.Integer" "parseInt" nid) node)
     (setAttr node "type" "occ")
     (setAttr node "nid" nid)
     (setAttr node "chattylabel" clab)
     (setAttr node "status" status)
     (setAttr node "init" (if (= status "init") "true" "false"))
     (setAttr node "subnet" subnet)
     (invoke graph "addNode" node)
     node))
) ;addONode

; add a rule node to given graph
(define addRNode (graph nodearray rlab ruleid nid subnet)
  (let ((node (object ("pla.graph.PLANode"
		       (object ("pla.data.Rule" 
				rlab 
				(sinvoke "java.lang.Integer" "parseInt" nid)
				ruleid))
		       "box"
		       (apply getNodeBorderColor (= subnet "true"))
		       (apply getRuleFillColor "none" (= subnet "true")))))
	)
    (seq 
     (aset nodearray (sinvoke "java.lang.Integer" "parseInt" nid) node)
     (setAttr node "type" "rule")
     (setAttr node "nid" nid)
     (setAttr node "ruleid" ruleid)
     (setAttr node "status" "none")
     (setAttr node "subnet" subnet)
     (invoke graph "addNode" node) 
     node))
) ;addRNode

; add a rule node to given graph with prepost ids
(define addRNodeX (graph nodearray rlab ruleid nid subnet pre post)
  (let ((node (apply addRNode graph nodearray rlab ruleid nid subnet)))
    (seq
      (setAttr node "prestr" pre)
      (setAttr node "poststr" post)
  ))
)

; add a unidirectional edge to given graph
(define addUniEdge (graph nodearray srcix tgtix subnet)
  (let ((src (aget nodearray srcix))
        (tgt (aget nodearray tgtix))
        (e (object ("g2d.graph.IOPEdge" src tgt 
		    (apply getEdgeColor "plain" (= subnet "true"))))) )
    (seq 
     (invoke e "setDoubleEnded" (boolean false))
     (invoke graph "addEdge" e)
     e))
) ;addUniEdge

; add a bidirectional edge to given graph
(define addBidirEdge (graph nodearray srcix tgtix subnet)
  (let ((src (aget nodearray srcix))
        (tgt (aget nodearray tgtix))
        (e (object ("g2d.graph.IOPEdge" src tgt 
		    (apply getEdgeColor "bidir" (= subnet "true"))))))
    (seq 
; uncomment the next line to make the edge dotted 
; can replace "dotted" by "dashed"
     (invoke e "setStyle" "dashed")
; replace false by true to get arrows at both ends
     (invoke e "setDoubleEnded" (boolean false))
     (invoke graph "addEdge" e) 
     e))
) ;addBidirEdge

;;; *********** implementations *********

; -----
; sendDisplayPetri:
; send message to Maude to build petri net from dish name
; -----
(define sendDisplayPetri (frame dish)
  (seq
   (invoke frame "setDishName" dish)
   (apply sendMessage "maude" 
	  "displayPetri2F" 
	  (concat (invoke (invoke frame "getID") "getUID") 
		  " " dish))
   (invoke frame "showProgressDialog" 
	   (concat "Loading dish \"" dish "\"..."))
   )
) ;sendDisplayPetri

; -----
; makeFrame:
; a common routine to be applied to all new frames
; returns newly created frame
; -----
(define makeFrame (frame)
  (seq
   ; make frame visible and display at front
   (invoke frame "setVisible" (boolean true))
   (invoke frame "toFront")
   frame)
) ;makeFrame

;;; *********** bootstrapping *********

; define exception handler
(sinvoke "g2d.jlambda.Debugger" "setHandler" (object ("pla.ExceptionHandler")))

; fills "Dishes" menu with predefined ones in every new PLAMainFrame
; uses file located in current directory: predef-dishes.lsp (if file exists)
(let ((file (object ("java.io.File" "predef-dishes.lsp"))))
  (if (and (!= file (object null)) 
	   (invoke file "canRead"))
      (seq
       (load "predef-dishes.lsp")
       (sinvoke "pla.PLAMainFrame" "setOpenDishClosure"
		(lambda (frame dish)
		  ; if the frame is empty, open petri net in this frame, 
		  ; otherwise, spawn a new frame
		  (if (invoke frame "isEmpty")
		      (apply sendDisplayPetri frame dish) ;then
		    (apply sendDisplayPetri 
			   (apply makeFrame (object ("pla.PLAMainFrame" ""))) 
			   dish) ;else
		    ))) ;if;lambda;sinvoke
       (sinvoke "pla.PLAMainFrame" "setPredefinedDishes" dishes)
       )) ;seq;if
) ;let

; add shutdown hook to send shutdown command to IOP
(invoke (sinvoke "java.lang.Runtime" "getRuntime") 
	"addShutdownHook" 
	(object ("pla.ShutdownHook" "shutdown_hook")))

; bring up empty, first PLA window
(apply makeFrame (object ("pla.PLAMainFrame" "")))

)