(seq
;;;;;;;;;; /Users/clt/bin/makeg2dlib loaded /Users/clt/Maude/Lib/M2.2/PLA1/G2dLib/pla.lsp ;;;;;;;;;;
(seq

;;; *********** DEBUG settings *********

; define verbosity
; (supdate "g2d.util.ActorMsg" "VERBOSE" (boolean true))
(supdate "pla.ExceptionHandler" "SHUTDOWN_AFTER_ERROR" (boolean false))
(sinvoke "g2d.jlambda.Debugger" "setVerbosity" (boolean true))

; if true prints lola retcode and path to GUI output
; and leaves the lola files in /tmp
(define lolaDebug (boolean false))

;;; *********** exception handling *********

; define exception handler
(sinvoke "g2d.jlambda.Debugger" "setHandler" (object ("pla.ExceptionHandler")))

; add shutdown hook to send shutdown command to IOP
(invoke (sinvoke "java.lang.Runtime" "getRuntime") 
	"addShutdownHook" 
	(object ("pla.ShutdownHook" "shutdown_hook")))

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
;(define ruleFillColor (object ("java.awt.Color" (int 251) (int 255) (int 145))))
(define ruleFillColor java.awt.Color.lightGray)
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

; called whenever node attribute changes to recalculate fill color:
(define calcNodeFillColor (nodename)
  (let ((node (fetch nodename)))
    (seq
     (if (= node (object null)) noneFillColor ; unknown node
     (if (= "rule" (getAttr node "type")) 
         (if (= "avoid" (getAttr node "status"))
             avoidFillColor
             ruleFillColor) ; "type" = "rule": look whether "avoid"
     (if (= "true" (getAttr node "context")) cxtFillColor
     (if (= "none" (getAttr node "status"))
         (if (= "true" (getAttr node "init")) 
             initFillColor
             noneFillColor) ; "status" = "none": look at "init"
         (if (= "goal" (getAttr node "status")) ; "status" != "none" 
             goalFillColor
             (if (= "avoid" (getAttr node "status")) avoidFillColor))
     )))) ; 4x if
  )) ; seq;let
) ;calcNodeFillColor	  



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
   (sinvoke "pla.PLABaseFrame" "hideProgressDialog")
   ; show message in warning dialog:
   (sinvoke "javax.swing.JOptionPane" "showMessageDialog" 
	    (object null)
	    msg
	    title
	    javax.swing.JOptionPane.WARNING_MESSAGE)
   )
)

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

(define defineGraph (gname kbname unrch makeGraph)
  (let ((graph (object ("pla.graph.PLAGraph"))))
    (seq
     ; make graph 
     (apply makeGraph graph)
     (invoke graph "setUID" gname)
     (setAttr graph "kbname" kbname)
     (setAttr graph "unRchGoals" unrch)
     graph))
) ; defineGraph

;;; --- adding nodes and edges to graph:

(define maxLabLen (int 20))
(define useChattyLabels (boolean false))

(define addOccNode (graph lab loc clab nid init subnet)
  (apply addOccNodeX graph lab loc clab nid init "none" subnet)
) ;addOccNode

;;; clt 06oct09 for setting status correctly
(define addOccNodeX (graph lab loc clab nid init status subnet)
  (let ((label (if useChattyLabels
                   clab
                	(if (> (invoke lab "length") maxLabLen)
				          (invoke lab "substring" (int 0) maxLabLen) 
				          lab)
                   ))
        (node (object ("pla.graph.PLANode"
                       (object ("pla.data.Occurrence"
                                label
                                (sinvoke "java.lang.Integer" "parseInt" nid)
                                clab))
                       "ellipse"
                       nodeBorderColor
                       (if (= init "true") initFillColor noneFillColor))))
	 )
    (seq 
     (setAttr node "type" "occ")
     (setAttr node "nid" nid)
     (setAttr node "loc" loc)
     (setAttr node "chattylabel" clab)
     (setAttr node "status" status)
     (setAttr node "init" init)
     (setAttr node "subnet" subnet)
     (invoke graph "addNode" node)
     node))
) ;addOccNodeX

; add a rule node to given graph
(define addRuleNode (graph lab clab nid subnet)
  (let ((node (object ("pla.graph.PLANode"
		       (object ("pla.data.Rule" 
				lab 
				(sinvoke "java.lang.Integer" "parseInt" nid)
				clab))
		       "box"
		       nodeBorderColor
		       ruleFillColor)))
	)
    (seq 
     (setAttr node "type" "rule")
     (setAttr node "nid" nid)
     (setAttr node "chattylabel" clab)
     (setAttr node "status" "none")
     (setAttr node "subnet" subnet)
     (invoke graph "addNode" node) 
     node))
) ;addRuleNode

; add a rule node to given graph with prepost ids
(define addRuleNodeX (graph lab clab nid subnet pre post)
  (let ((node (apply addRuleNode graph lab clab nid subnet)))
    (seq
      (setAttr node "prestr" pre)
      (setAttr node "poststr" post)
  ))
)

; add a unidirectional edge to given graph
(define addUniEdgeX (graph srcid tgtid subnet)
  (let ((src (invoke graph "getNode" srcid))
        (tgt  (invoke graph "getNode" tgtid))
        (e (object ("g2d.graph.IOPEdge" src tgt unidirEdgeColor))) )
    (seq 
     (invoke e "setDoubleEnded" (boolean false))
     (invoke graph "addEdge" e)
     e))
) ;addUniEdgeX

; add a bidirectional edge to given graph
(define addBidirEdgeX (graph srcid tgtid subnet)
  (let ((src (invoke graph "getNode" srcid))
        (tgt  (invoke graph "getNode" tgtid))
        (e (object ("g2d.graph.IOPEdge" src tgt bidirEdgeColor))) ) 
    (seq 
; can replace "dotted" by "dashed"
     (invoke e "setStyle" "dashed")
; replace false by true to get arrows at both ends
     (invoke e "setDoubleEnded" (boolean false))
     (invoke graph "addEdge" e) 
     e))
) ;addBidirEdgeX

;;; *********** begin explorer code *********

;;; 06oct09 clt code for exploring 

(define colorXGraph (graph)
  (let ((nodes (invoke graph "getNodesInArray")))
     (for node nodes
       (if (= (getAttr node "type" "") "rule")
         (invoke node "setFillColor" java.awt.Color.lightGray)
         (let ((xstatus (getAttr node "xstatus" ""))
               (color 
                 (if (= xstatus "seen")
                     java.awt.Color.lightGray
                 (if (= xstatus "oup") initFillColor
                 (if (= xstatus "odn") java.awt.Color.green
                 (if (= xstatus "oboth") java.awt.Color.cyan 
                  java.awt.Color.white)
                 ))))  ; color
             )
          (seq 
            (invoke node "setFillColor" color)
          )
         ) ;let
       ) ;if
     ) ;for
  ) ;let
)

(define redisplay (graph)
  (let ((frame (getAttr graph "frame" (object null))))
     (seq
      (invoke graph "resetDotLayout")
      (if (invoke graph "isDotLayout") (invoke graph "doLayout" (object null)))
      (invoke frame "setGraph" graph)
      (invoke frame "repaint") ;; redundant setGraph repaints
      )
   )
)

; (apply addXOccNode graph %occLab %occLoc %occChattyLab %nodeId %occXInit %occXStatus)
(define addXOccNode (graph lab loc clab nid xinit xstatus)
  (let ((node (apply addOccNodeX graph lab loc clab nid xinit "none" "true"))
	)
    (seq
     (setAttr node "init" "false") ; override setting of "xinit" used for coloring
     (setAttr node "xinit" xinit)
     (setAttr node "xstatus" xstatus)
     node))
) ;addXOccNode

;;; *********** implementations *********

; -----
; resolveID:
; helper function to resolve given "fname" IDs
; returns given "obj" if "fname" is invalid or corresponding frame doesn't exist
; returns retrieved frame otherwise
; -----
(define resolveID (fname obj)
  (let ((frameID (if (!= fname (object null))
		     (fetch fname)
		     (object null)))
	(frame (if (instanceof frameID "pla.IDFrame$ID")
		   (invoke frameID "getFrame")
		   obj))
	)
    frame)
) ; resolveID

; -----
; resolveGraph:
; helper function to resolve given "gname"
; returns given "obj" if "gname" is invalid or corresponding graph doesn't exist
; returns retrieved graph otherwise
; -----
(define resolveGraph (gname obj)
  (let ((graph (if (instanceof gname "java.lang.String")
		     (fetch gname)
		     (object null))))
    (if (instanceof graph "pla.graph.PLAGraph")
	graph
        obj))
) ; resolveGraph
    
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


(define defKBG (kbname occ-labs occ-ids rule-labs rule-ids)
  (let ((kbg0 (fetch kbname))
        (kbg (if (= kbg0 (object null))  ; **** don't create if exists
                 (object ("g2d.glyph.Attributable"))
                 kbg0)))
    (seq
      (if (= kbg0 (object null)) (invoke kbg "setUID" kbname))
      (setAttr kbg "frame" (object ("g2d.swing.IOPFrame" "test")))
      (setAttr kbg "occ-labs" occ-labs)
      (setAttr kbg "occ-ids" occ-ids)
      (setAttr kbg "rule-labs" rule-labs)
      (setAttr kbg "rule-ids" rule-ids)
    )
  ))

)
;;;;;;;;;; /Users/clt/bin/makeg2dlib loaded /Users/clt/Maude/Lib/M2.2/PLA1/G2dLib/exploreSelect.lsp ;;;;;;;;;;
(seq
; (apply exploreSelectRules %title %gname %rids %replyto %replyfor)

;                             graphname array[String]
(define exploreSelectRules (title parent ruleids replyto replyfor)
  (let ((frame (getAttr (fetch parent) "frame" (object null)))
        (sdialog (object ("g2d.subset.SDialog" frame (boolean true))) )
       )
    (seq
      (invoke sdialog "setUniverse" ruleids)
      (invoke sdialog "addTab" "Lexical")
      (invoke sdialog "setTitle" title)
      (invoke sdialog "setVisible" (boolean true))
      (invoke sdialog "toFront")
      (let ((selected (invoke sdialog "getStrings" (int 1)))
            (ans (if (= selected (object null)) 
                   ""
                  (apply sarray2string  selected 
                               (lookup selected "length") (int 0) "")
                 ))
             )
        (sinvoke "g2d.util.ActorMsg" "send" replyto replyfor ans)
       )      
     ))  ; seq ; let
) ; end exploreSelectRules

(define sarray2string (selected len cur ans)
  (if (>= cur len)
   ans
   (apply sarray2string  selected len (+ cur (int 1))
          (concat ans " " (aget selected cur))      
   ))
)

; (apply sarray2string (array java.lang.String "a" "b" "c") (int 3) (int 0) "")

; (apply exploreSelectOccs %title %gname %occids %replyto %replyfor)

(define exploreSelectOccs (title parent occids replyto replyfor)
  (let ((frame (getAttr (fetch parent) "frame" (object null)))
        (sdialog (object ("g2d.subset.SDialog" frame (boolean true))) )
        (four (object ("g2d.subset.StateSpace" )))
        (names (array java.lang.String 
                      "         " 
                      " (both)  "
                      " (up)    "
                      " (dn)    "))
        (dummy (seq (invoke four "setValency" (int 4))
                    (invoke four "setNames" names)))
        (universe (object ("g2d.subset.Universe" occids four)))
       )
    (seq
      (invoke sdialog "setUniverse" universe)
      (invoke sdialog "addTab" "Lexical")
      (invoke sdialog "setTitle" title)
      (invoke sdialog "setVisible" (boolean true))
      (invoke sdialog "toFront")
      (let ((both (invoke sdialog "getStrings" (int 1)))
            (up (invoke sdialog "getStrings" (int 2)))
            (dn (invoke sdialog "getStrings" (int 3)))
            (ansb (if (= both (object null)) 
                   ""
                  (apply occChattySelect2string  both "b" 
                               (lookup both "length") (int 0) "")))
            (ansu (if (= up (object null)) 
                   ""
                  (apply occChattySelect2string  up "u" 
                               (lookup up "length") (int 0) (concat ansb " "))))
            (ansd (if (= dn (object null)) 
                   ""
                  (apply occChattySelect2string  dn "d" 
                               (lookup dn "length") (int 0) (concat ansu " "))))
              )
        (sinvoke "g2d.util.ActorMsg" "send" replyto replyfor ansd)
       )      
     ))  ; seq ; let
) ; end exploreSelectRules


(define occChattySelect2string (selected tag len cur ans)
  (if (>= cur len)
   ans
   (apply occChattySelect2string  selected tag len (+ cur (int 1))
          (concat ans  (aget selected cur) " " tag " " )      
   ))
)




(define test (what)
  (let ((parent (object ("g2d.glyph.Attributable")))
        (frame (object ("g2d.swing.IOPFrame" "test")))
        (ids (array java.lang.String
                         "102.def"  "hello" "Src-act-CLi" 
                           "1433-CLc" "213.xyz#a"))
        )
    (seq
      (invoke parent "setUID" "parent") 
      (setAttr parent "frame" frame)
      (if (= what "rules")
        (apply exploreSelectRules "foo" "parent" ids "maude" "maudereq3")
        (apply exploreSelectOccs "foo" "parent" ids "maude" "maudereq3")
      )
    ) ; seq
  ) ; let
)
; (apply test "rules")
; (apply test "occs")
)
;;;;;;;;;; /Users/clt/bin/makeg2dlib loaded /Users/clt/Maude/Lib/M2.2/PLA1/G2dLib/showing.lsp ;;;;;;;;;;
(seq
(define mkExploreInitAction (kbname mode label tip)
  (let ((eiClosure
          (lambda (self event)
             (sinvoke "g2d.util.ActorMsg" "send" 
                      "maude" kbname (concat "exploreInit " mode))) 
        ))
    (object ("g2d.closure.ClosureAbstractAction"
             label
          	 (object null) ; icon
         	 tip
         	 (object null) ; accelerator
         	 (object null) ; mnemonic
             eiClosure ))
   )
) ; mkExploreInitAction

(define getMenu (dm text)
  (let ((mb (invoke dm "getOrigJMenuBar"))
        (num (invoke mb "getMenuCount"))
        )
    (apply getMenuAux mb num (int 0) text)
))    

(define getMenuAux (mb num cur text)
  (if (>= cur num)
   (object null)  ; not found
   (let ((menu (invoke mb "getMenu" cur)))
     (if (= (invoke menu "getText") text)
      menu
     (apply getMenuAux mb num (+ cur (int 1)) text)
     )) ; if let
   ) ; if
)
; common graph showing code
(define anyShowGraph (graph frame title subtitle)
   (seq
     (invoke graph "setStrokeWidth" (float 1.0))
     ; do layout with dot by default
     (invoke graph "doLayout" (object null))
     ; update frame and set graph into frame
   ; make frame visible and display at front
   ;  replacing   (apply makeFrame frame)
     (invoke frame "setVisible" (boolean true))
     (invoke frame "toFront")
     (invoke frame "setTitle" title)
     (invoke frame "setGraph" graph)
     (invoke frame "setSubtitle" subtitle)
     graph)
) ; anyShowGraph

; need to setup explorer frame not MainFrame
(define showXGraph (gname pname title subtitle update?)
  (let ((graph (if (instanceof gname "java.lang.String")
            		 (fetch gname)
                   (object null)))
        )
    (if (instanceof graph "pla.graph.PLAGraph")
        (seq 
          (if (= update? "true")
              (apply redisplay graph)
              (seq (apply colorXGraph graph)
                   (apply exploreGraph gname graph pname title subtitle))
          ) ; if
        )) ; seq ; if
  ) ; let
) ; showXGraph


; !!! need to look for non empty unRchGoals attr
; show graph in frame given by fname or in newly created frame, if necessary
(define showGraph (gname title subtitle fname)
  (let ((kbframe (apply resolveID (getAttr (fetch "KBManager") "kbfname" "") (object null)))
        (frame (apply resolveID fname (object ("pla.PLABaseFrame" "" kbframe (boolean true)))))
      	(graph (apply resolveGraph gname (object ("pla.graph.PLAGraph"))))
      	(toolbar (invoke frame "getToolBar"))
	      )
    (seq
  ; prepend buttons and things in tool bar
     (invoke toolbar "prepend" 
             (sinvoke "pla.toolbar.SeparatorFactory" "makeLargeSep"))
;     (invoke toolbar "prepend" (invoke frame "getLayoutButton"))
     (invoke toolbar "prepend" 
             (sinvoke "pla.toolbar.SeparatorFactory" "makeLargeSep"))
     (invoke toolbar "prepend" 
        (object ("pla.toolbar.ToolButton"
          (apply mkAction "FindPath" "find a path to goals" 
             (lambda (self event)
                (apply pathRequest graph))) ) ) )
     (invoke toolbar "prepend" 
        (object ("pla.toolbar.ToolButton"
          (apply mkAction "Subnet" "display relevant subnet" 
             (lambda (self event)
                (apply subnetRequest graph ))) ) ) )
     (invoke toolbar "prepend" 
             (sinvoke "pla.toolbar.SeparatorFactory" "makeLargeSep"))
     (invoke toolbar "prepend" 
        (object ("pla.toolbar.ToolButton"
           (apply mkExploreInitAction gname "rule"
                  "InitExplore(Rules)"
                  "Explore starting from selected rules") )))
     (invoke toolbar "prepend" 
        (object ("pla.toolbar.ToolButton"
           (apply mkExploreInitAction gname "occ"
                  "InitExplore(Occs)"
                  "Explore starting from selected occurences") )))
  ; Now do the graph showing
      (apply anyShowGraph graph frame title subtitle)
     )) ; seq ; let
) ; showGraph

(define mkAction (label tip closure)
    (object ("g2d.closure.ClosureAbstractAction"
             label
				 (object null) ; icon
				 tip
             (object null) ; accelerator
				 (object null) ; mnemonic
				 closure     ; action closure
) ) )

(define mkExploreClosure (gname cb cmd)
     (lambda (self event)
        (let ((new (invoke cb "isSelected")))
           (seq (sinvoke "g2d.util.ActorMsg" "send" "maude" gname 
                      (concat "explore " new " " cmd))
                (invoke cb "setSelected" (boolean false))
            )
         )
     ) ; exploreClosure
)

(define addExploreButton (toolbar gname cb)
  (let ((button (object ("g2d.swing.IOPDropdownButton" "Explore")))
        (fpsC (apply mkExploreClosure gname cb "fps"))
        (addRC (apply mkExploreClosure gname cb "addR"))
        (hideRC (apply mkExploreClosure gname cb "hideR"))
        (unhideRC (apply mkExploreClosure gname cb "unhideR"))
        (ht pla.toolbar.ToolBar.TOOL_BTN_HEIGHT)
   )
 (seq
   (invoke button "addMenuItem" "occs" fpsC)
   (invoke button "addMenuItem" "add Rules" addRC)
   (invoke button "addMenuItem" "hide Rules" hideRC)
   (invoke button "addMenuItem" "unhide Rules" unhideRC)
   (invoke button "setHeight" ht)
   (invoke toolbar "add"  button (int 0))
 ))
) ; addExploreButton


; initialize frame and show new graph to explore:
; pgname: parent graph ID (or NULL if not anchored)
(define exploreGraph (gname graph pgname title subtitle)
  (let ((pgraph (apply resolveGraph pgname (object null)))
        (pframe (if (instanceof pgraph "pla.graph.PLAGraph")
		              (getAttr pgraph "frame")
          		    (object null)))
        (kbframe (apply resolveID 
                        (getAttr (fetch "KBManager") "kbfname" "") (object null)))
        (frame (if (!= pframe (object null))
                   (object ("pla.PLABaseFrame" "" pframe (boolean false)))
                   (object ("pla.PLABaseFrame" "" kbframe (boolean false)))))
        (toolbar (invoke frame "getToolBar"))
        (cb (object ("pla.toolbar.ToolCheckBox" "New Frame")))
        (tf (object ("pla.toolbar.ToolTextField")))
       )
    (seq
     ; prepend buttons and things in tool bar
     (invoke toolbar "prepend"
             (sinvoke "pla.toolbar.SeparatorFactory" "makeLargeSep"))
     ; check box for new or reuse frames
     (invoke cb "setToolTipText"
                "Open graph resulting from next explore operation in new frame")
     (invoke toolbar "prepend" cb)
     (invoke toolbar "prepend" 
             (sinvoke "pla.toolbar.SeparatorFactory" "makeSmallSep"))
     ; text field for number of steps
     (invoke tf "setToolTipText" 
                "Specify number of steps to be taken when exploring up or down")
     (invoke toolbar "prepend" tf)
     (invoke toolbar "prepend" 
                     (sinvoke "pla.toolbar.SeparatorFactory" "makeSmallSep"))
     ; up and down buttons
     (let ((dnClosure 
           (lambda (self event)
             (let ((new (invoke cb "isSelected"))
                   (steps (invoke (invoke tf "getValue") "intValue"))
                  )
            (seq (sinvoke "g2d.util.ActorMsg" "send" "maude" gname 
                      (concat "explore " new " dn " steps))
                  (invoke cb "setSelected" (boolean false)))
            ))) ; dnClosure
           (label "Down")
           (tip "Explore down given steps")
         ) ; letbindings  				 
      (invoke toolbar "prepend"
        (object ("pla.toolbar.ToolButton" (apply mkAction label tip dnClosure))))
      ) ; let                      
;    (invoke toolbar "prepend"
;                    (sinvoke "pla.toolbar.SeparatorFactory" "makeSmallSep"))
    (let ((upClosure 
           (lambda (self event)
             (let ((new (invoke cb "isSelected"))
                   (steps (invoke (invoke tf "getValue") "intValue")))
             (sinvoke "g2d.util.ActorMsg" "send" "maude" gname 
                      (concat "explore " new " up " steps)))
            )) ; upClosure
           (label "Up")
           (tip "Explore up given steps")
         )  				 
      (invoke toolbar "prepend"
        (object ("pla.toolbar.ToolButton" (apply mkAction label tip upClosure))))
     ) ; let                      
     ; explore dropdown button
     (apply addExploreButton toolbar gname cb)
    ; show graph in frame
     (apply anyShowGraph graph frame title subtitle)
  ))
) ; exploreGraph
)
;;;;;;;;;; /Users/clt/bin/makeg2dlib loaded /Users/clt/Maude/Lib/M2.2/PLA1/G2dLib/query.lsp ;;;;;;;;;;
(seq

(define mkStatusString (graph)
  (let ((nodes (invoke graph "getNodesInArray")))
    (apply nodes2status nodes (int 0) (lookup nodes "length") "")
  ) 
)

(define nodes2status (nodes cur len str)
  (if (>= cur len)
   str
   (let ((node (aget nodes cur))
         (nid (getAttr node "nid" ""))
         (status (getAttr node "status" "none")))
    (apply nodes2status nodes (+ cur (int 1)) len 
         (if (or (= status "none") (= nid ""))
          str 
          (concat str " " nid " " status) 
      ) ) ) ; if app let 
  )  
) ; nodes2status


(define subnetRequest (graph)
   (sinvoke "g2d.util.ActorMsg" "send" 
      "maude"
       (invoke graph "getUID")
       (concat "displaySubnet1" " " (apply mkStatusString graph))
    )
 )

(define pathRequest (graph)
   (sinvoke "g2d.util.ActorMsg" "send" 
      "maude"
       (invoke graph "getUID")
       (concat "displayPath1" " " (apply mkStatusString graph))
 ) )
 

(define nodeById (nodes id cur len)
  (if (>= cur len)
   (object null)
   (let ((node (aget nodes cur))
         (nid (getAttr node "nid" ""))
        )
     (if (= nid id)
      node
      (apply nodeById nodes id (+ cur (int 1)) len) 
      ) ) ) ; if let if
) ; nodeById

(invoke java.lang.System.err "println"  "query.lsp loaded")
)

;;;;;;;;;; /Users/clt/bin/makeg2dlib loaded /Users/clt/Maude/Lib/M2.2/PLA1/G2dLib/kbmanager.lsp ;;;;;;;;;;
(seq

(define mkKBAction (label tip list cmd)
  (let ((closure
           (lambda (self event)
                (let ((selection (invoke list "getSelectedValue")))
                  (if (= selection (object null))
                      (invoke java.lang.System.err "println" 
                           (concat "KBManager no kb selected for " cmd)) 
                      (sinvoke "g2d.util.ActorMsg" 
                               "send" "maude" selection cmd)))))
         )
    (object ("g2d.closure.ClosureAbstractAction"
             label
				 (object null) ; icon
				 tip
             (object null) ; accelerator
				 (object null) ; mnemonic
				 closure     ; action closure
            ))
   )) ;mkKBAction
;                      dish name string array
(define initKBwdo (title kbm dishes)
  (let ((frame (object ("pla.KBManagerFrame" title)))
        (model (object ("javax.swing.DefaultListModel")))
        (list (object ("javax.swing.JList" model) ))
        (toolbar (object ("javax.swing.JToolBar"  javax.swing.JToolBar.VERTICAL)))
;;; added
        (dishbutton (object ("g2d.swing.IOPDropdownButton" "Select Dish")))
        (editDishClosure
           (lambda (self event)
              (let ((selection (invoke list "getSelectedValue")))
                  (sinvoke "g2d.util.ActorMsg" 
                               "send" "maude" selection "newDish"))))
        (predefClosure (lambda (dname) (lambda (s e )
                (let ((kbname  (invoke list "getSelectedValue")))
                  (sinvoke "g2d.util.ActorMsg" 
                    "send" "maude" kbname (concat "displayPetri " dname)) )) ))
;;omitthis
        (newDishAction 
           (apply mkKBAction "newDish" 
                            "start dish editor for selected knowledge base" 
                             list "newDish"))
        (exploreOccsAction 
           (apply mkKBAction "Explore(Occs)" 
                           "Explore the selected knowledge base from occurrences" 
                             list "exploreInit occ"))
        (exploreRulesAction 
           (apply mkKBAction "Explore(Rules)" 
                           "Explore the selected knowledge base from rules" 
                             list "exploreInit rule"))
         )
    (seq
      (invoke frame "setSize" (int 200) (int 350))
      (invoke list "setFixedCellHeight" (int 20))
      (invoke toolbar "addSeparator")
;      (invoke toolbar "add" newDishAction)
;; replace above by next 3
      (invoke dishbutton "addMenuItem" "Edit" editDishClosure)
      (if (> (lookup dishes "length") (int 0))
          (invoke dishbutton "addMenu" "PreDefined" dishes predefClosure))
      (invoke toolbar "add" dishbutton (int -1))
;;;
      (invoke toolbar "addSeparator")
      (invoke toolbar "add" exploreOccsAction)
      (invoke toolbar "addSeparator")
      (invoke toolbar "add" exploreRulesAction)
      (invoke toolbar "setFloatable" (boolean false))
      (invoke frame  "add" toolbar java.awt.BorderLayout.EAST)
      (invoke frame  "add" list java.awt.BorderLayout.CENTER)
      (setAttr kbm "kbnames" model)
      (setAttr kbm "kblist" list)
      (setAttr kbm "kbframe" frame)
;; redundant
      (setAttr kbm "kbfname" (invoke (invoke frame "getID") "getUID"))
      (invoke frame "setVisible" (boolean true))
) ) ) ; seq ; let ; initKBwdo

(define defKBManager ()
  (let ((name  "KBManager")
        (kbm0 (fetch name))
        (kbm (if (= kbm0 (object null))  ; **** don't create if exists
                 (object ("g2d.glyph.Attributable"))
                 kbm0))
        (dishes (array java.lang.String ))
     )
    (seq
      (if (= kbm0 (object null)) (invoke kbm "setUID" name))
      (apply initKBwdo "PLA KB Manager" kbm dishes) ;; set kbnames,kbframe attrs.
    )
))

(define defKBManagerD (dishes)
  (let ((name  "KBManager")
        (kbm0 (fetch name))
        (kbm (if (= kbm0 (object null))  ; **** don't create if exists
                 (object ("g2d.glyph.Attributable"))
                 kbm0)))
    (seq
      (if (= kbm0 (object null)) (invoke kbm "setUID" name))
      (apply initKBwdo "PLA KB Manager" kbm dishes) ;; set kbnames,kbframe attrs.
    )
))

(define defKBGraph (kbname occ-labs occ-ids occ-locs rule-labs rule-ids)
  (let ((kbg0 (fetch kbname))
        (kbg (if (= kbg0 (object null))  ; **** don't create if exists
                 (object ("g2d.glyph.Attributable"))
                 kbg0))
        (kbm (fetch "KBManager"))
        (kbnames (if (= kbm (object null)) 
                     (object null)
                     (getAttr kbm "kbnames" (object null))))
        (kblist (if (= kbm (object null)) 
                     (object null)
                     (getAttr kbm "kblist" (object null))))
         )
    (seq
      (invoke kbnames "addElement" kbname)  ; add element to list
      (if (= kbg0 (object null)) (invoke kbg "setUID" kbname))
      (setAttr kbg "frame" (object ("g2d.swing.IOPFrame" "test")))
      (setAttr kbg "occ-labs" occ-labs)
      (setAttr kbg "occ-ids" occ-ids)
      (setAttr kbg "occ-locs" occ-locs)
      (setAttr kbg "rule-labs" rule-labs)
      (setAttr kbg "rule-ids" rule-ids)
      (if (invoke kblist "isSelectionEmpty")
          (invoke kblist "setSelectedIndex" (int 0)))
    )
  ))

(define askUser (frame title msg)
   (let ((asker (object ("g2d.swing.IOPAskUser" frame title msg (boolean true)))))
      (seq 
        (invoke asker "setVisible" (boolean true))
        (invoke asker "getAnswer")
      )
     )
  )

; (apply initDishEditor %kbname %dishnames)
(define initDishEditor (kbname dishnames)
  (let ((kb (fetch kbname))
         (entries (if  (instanceof kb "g2d.glyph.Attributable")
                       (getAttr kb "occ-labs")
                       (array java.lang.String) ))
         (locations (if (instanceof kb "g2d.glyph.Attributable")
                        (getAttr kb "occ-locs")
                        (array java.lang.String) ))
        (frame (object ("javax.swing.JFrame" "DDialog Test")))
;                                                         modal?
        (ddialog (object ("g2d.subset.DDialog" frame (boolean false))))
        (button (object ("g2d.swing.IOPDropdownButton" "Dish")))
      ; the two closures for the drop down button:
        (open (lambda (s e) 
	        (let ((dish (invoke ddialog "getEntriesFromFile")))
            	  (invoke ddialog "add2Dish" dish))))
       (save (lambda (s e) (invoke ddialog "saveEntriesToFile")))
       (askmaude (lambda (dname) (lambda (s e )
           (sinvoke "g2d.util.ActorMsg" 
                    "send" "maude" kbname (concat "getDish " dname)) )) )
       (okClosure 
         (lambda (s e)
           (let ((selected (invoke ddialog "getSelected"))
                 (udname (apply askUser frame "AskUser" "Type in a dish name"))
                 (ans (apply sarray2string selected 
                             (lookup selected "length") (int 0) "")))
               (sinvoke "g2d.util.ActorMsg" "send" "maude" kbname 
                              (concat "displayNewDish " udname " " ans))
           )))
        )
  (seq 
   (invoke ddialog "add2DishToolbar" button)

   (invoke button  "addMenuItem" "Open" open)
   (invoke button  "addMenuItem" "Save" save)
   (invoke button  "addMenu" "Ask Maude" dishnames askmaude)

   (invoke ddialog "setScope" entries)

   ;make a  real tab via the agreed API
   (invoke ddialog "classify" "Spatial" entries  locations)
   (invoke ddialog "addTab" "Spatial")

   ;now actually build the trees 
;   (invoke ddialog "fireUpdate")
   (if (instanceof kb "g2d.glyph.Attributable") 
       (setAttr kb "dishEditor" ddialog))
   (invoke ddialog "setOKClosure" okClosure)
   (invoke ddialog "setVisible" (boolean true))
   ))
) ;initDishEditor

; maude kbname displayNewDish user-dname toks

; want to give disheditor closure to call upon exit
; also need to implement getDish and user-dname
; initDishEditor needs to return before the dishEditor does
; KBManager needs a PLA quit button

; (apply getDishReply %kbname %dishoccs)
;                      string String[]
(define getDishReply (kbname dishoccs)
  (let ((kb (fetch kbname))
        (editor (if (instanceof kb "g2d.glyph.Attributable")
                    (getAttr kb "dishEditor" )
                    (object null)))
       )                
    (if (instanceof editor "g2d.subset.DDialog")
        (invoke editor "add2Dish" dishoccs))
  )
)
  
) ;seq

; (apply defKBManager)
; (apply defKBGraph "KB0" (array java.lang.String "occ0") (array java.lang.String "0") (array java.lang.String "Out") (array java.lang.String "1.occ.act") (array java.lang.String "1") )

; (define kbl6 (getAttr (fetch "KBManager") "kblist"))
;;;;;;;;;; /Users/clt/bin/makeg2dlib loaded /Users/clt/Maude/Lib/M2.2/PLA1/G2dLib/info.lsp ;;;;;;;;;;
(seq
 
; *** create a string from an array of strings recursively
;     each string in array is prepended with given prefix, then the given 
;     closure <makeString> is applied (use "ident" if no change desired), 
;     and then given postfix (e.g., a newline character) is appended
; arr: Array of strings
; cur: current index in recursion
; msg: containing string assembled so far
(define printArray (arr prefix makeString postfix cur msg)
  (if (>= cur (lookup arr "length"))
      msg
      (apply printArray arr prefix makeString postfix (+ cur (int 1)) 
	     (concat msg 
		     prefix 
		     (apply makeString (aget arr cur))
		     postfix))
  )
)

; use this for "makeString" if strings in array don't need alteration
(define ident (str) str) ; identity function

;;; ------ requests from Maude -----

(define displayProteinInfo (gname hugosym spnum synonymsArray)
  (let ((message 
           (concat "Hugo: " hugosym "\n" 
                   "SwissProt ID: " spnum "\n"
                   "Synonyms: \n")))
    (apply displayMessage2G gname "ProteinInfo" 
      (apply printArray synonymsArray "    " ident "\n" (int 0) message))
   )
)

(define displayChemicalInfo (gname keggcpd synonymsArray)
  (let ((message 
           (concat "KeggCpd: " keggcpd "\n" 
                   "Synonyms: \n")))
    (apply displayMessage2G gname "ChemicalInfo" 
      (apply printArray synonymsArray "    " ident "\n" (int 0) message))
   )
)

(define displayOtherInfo (gname sort opstr)
  (let ((message 
           (concat "Sort: " sort "\n" 
                   "Occ:  " opstr "\n")))
    (apply displayMessage2G gname "OtherInfo" message)
   )
)

(define ruleEvidence (gname clab refArray)
  (apply displayMessage2G gname 
	 (concat "Evidence for Rule \"" clab "\"")
	 (if (> (lookup refArray "length") (int 0))
	     (concat "<html><br>"
		     (apply printArray refArray "PubMed ID " makePMIDLink "<br>" (int 0)  " ")
		     "</html>")
	     "\n(none)"))
)
(define makePMIDLink (pmid)
  (concat "<a href=\"http://www.ncbi.nlm.nih.gov/entrez/query.fcgi?cmd=Retrieve&db=PubMed&list_uids="
	  pmid
	  "&dopt=Abstract\">"
	  pmid
	  "</a>")
)

)
;;;;;;;;;; /Users/clt/bin/makeg2dlib loaded /Users/clt/Maude/Lib/M2.2/PLA1/G2dLib/infox.lsp ;;;;;;;;;;
(seq
(define displayComponentInfo (gname clab infoArray)
  (apply displayMessage2G gname 
     (concat "About " clab )
     (let ((strb (object ("java.lang.StringBuffer"))))
      (seq 
        (invoke strb "append" "<html><br> ")
        (for item infoArray (apply makeInfoItem item strb ))
        (invoke strb "append" " </html>")
        (invoke strb "toString")
      ) ) ; seq let
  ) ; apply
)


(define makeInfoItem (item strb)
 (if (> (lookup item "length") (int 2))
  (let ((type (aget item (int 0)))
        (tag (aget item (int 1))) )
   (if (= type "val")
     (invoke strb "append" (concat tag  ": "  (aget item (int 2)) " <br><p>"))  
     (if (> (lookup item "length") (int 3))
       (if (= type "link") 
         (invoke strb "append" (concat tag  ": <a href=\""  
                                       (aget item (int 3)) "\">"
                                       (aget item (int 2)) "</a><br><p>" )) 
         (if (= type "list")
           (seq 
             (invoke strb "append" (concat tag  ": <br><ul>"))
             (apply makeListItem (aget item (int 2)) (aget item (int 3)) strb)
             (invoke strb "append" "</ul>")
            ) ; 
           "" ) ; if list
         ) ; if link
      "") ; 3x if len > 3
     ) ; if val
   ) ; let
  ) ; if len > 2
)

(define makeListItem (fun items strb)
  (for item items (invoke strb "append" (concat " <li> " item)))
)

)

(seq
 (define a1 (array java.lang.String "val" "Name"  "foo"))
 (define a2 (array java.lang.String "link" "KEGG" "C00042"
                 "http://www.genome.jp/dbget-bin/www_bget?compound+C00042"))
 (define a3  (array java.lang.Object "list" "Synonyms"  ident 
              (array java.lang.String  "synonym1" "synonym2" )))
 (define iarr (array java.lang.Object a1 a2 a3))
 (define s1 "Name: foo <br><p>")
 (define s2 (concat "KEGG: "
  "<a href=\"http://www.genome.jp/dbget-bin/www_bget?compound+C00042\">"
	  "C00042 </a>" "<br><p>"
	))
 (define s3 "Synonyms: <br> <ul> <li> s1 <li> s2 </ul><br>")
 (define msg (concat "<html><br> " s1 s2 s3 " </html>"))
)
; (apply displayComponentInfo "graph4" "something" iarr)




;;;;;;;;;; /Users/clt/bin/makeg2dlib loaded /Users/clt/Maude/Lib/M2.2/PLA1/G2dLib/newpla.lsp ;;;;;;;;;;
(seq

(define setAttrsAVX (obj tags vals)
  (let ((tlen (lookup tags "length"))
        (vlen (lookup vals "length"))
        (len (if (> tlen vlen) vlen tlen))
      )
  (apply setAttrsAV obj tags vals len (int 0))
  )
)

(define setAttrsAV (obj tags vals len cur)
  (if (>= cur len)
   obj
   (seq
    (setAttr obj (aget tags cur) (aget vals cur))
    (apply setAttrsAV obj tags vals len (+ cur (int 1)))))
)

(define lookupAV (tags vals len cur tag default)
  (if (>= cur len)
   default
   (if (= (aget tags cur) tag)
    (aget vals cur)
    (apply lookupAV tags vals len (+ cur (int 1)) tag default)
   )
  )
)

(define colorPNode (type tags vals len)
  (if (= type "occ") 
    (let ((init (apply lookupAV tags vals len (int 0) "init" "false"))
          (status (apply lookupAV tags vals len (int 0) "status" "none"))
       )
    (if (= init "true")
     initFillColor
    (if (= status "none")
     noneFillColor
    (if (= status "goal")
      goalFillColor
    (if (= status "avoid")
      avoidFillColor
      noneFillColor
     ))))  ; 4x if
    ) ; let
  (if (= type "rule")  
    (let ((status (apply lookupAV tags vals len (int 0) "status" "none"))
       )
    (if (= status "none")
     ruleFillColor
    (if (= status "avoid")
      avoidFillColor
      ruleFillColor
     ))  ; 2x if
    ) ; let
  ruleFillColor ; shouldn't happen
  ))
)

(define colorCNode (type tags vals len)
  (let ((compare (apply lookupAV tags vals len (int 0) "compare" "both")))
    (if (= compare "left")  ; parent
      initFillColor
    (if (= compare "right")
      (object ("java.awt.Color" (int 0) (int 255) (int 255)))
      java.awt.Color.white
     ))  
    ) ; let
)


(define colorXNode (type tags vals len)
  (let ((xstatus (apply lookupAV tags vals len (int 0) "xstatus" "seen")))
    (if (= type "rule")
        (apply colorXRuleNode (boolean false) )
        (apply colorXOccNode xstatus (boolean false) "none")
     ))
)

(define colorXRuleNode (selected)
   (if selected 
     java.awt.Color.yellow
    java.awt.Color.lightGray)
)

(define colorXOccNode (xstatus selected mode)
   (if selected
    ;; dispatch on mode
     java.awt.Color.yellow  
   ;; dispatch on xstatus
     (if (= xstatus "seen") java.awt.Color.lightGray
     (if (= xstatus "oup") initFillColor
     (if (= xstatus "odn") java.awt.Color.green
     (if (= xstatus "oboth") java.awt.Color.cyan 
      java.awt.Color.white
      )))) ; 4x if xstatus
    )  ; if selected
)

(define extendSEMenu (graph  clist)
  (let (
        (frame (getAttr graph "frame"))
        (gp (invoke frame "getGraphPanel"))
        (gpf (invoke gp "getFrame"))
        (sep (invoke gpf "getSEPanel"))
      )
  (seq
;    (invoke java.lang.System.err "println"  "extendingSEMenu")
    (invoke sep "displayMenu" "" clist (boolean false))
  )
))

(define testEvent (self e)
  (let ((cb (object ("javax.swing.JCheckBox" "text next to box")))
        (ac (lambda (self e)
               (invoke java.lang.System.err "println"  cb)))
        (clist (object ("java.util.ArrayList")))
      )
     (seq (invoke cb "setAction"
           (object ("g2d.closure.ClosureAbstractAction"
                     "myaction" ac)))
          (invoke clist "add" cb)
          (apply extendSEMenu (fetch "graph10") clist)
     )))

(define resetXselect (graph)
  (let ((nodes (invoke graph "getNodesInArray")))
    (for node nodes (setAttr node "xselect" "none"))
  )
)

(define mkXRuleCheckClosure (frame node cb  mode)
  (lambda (self e)
     (let ((checked? (invoke cb "isSelected"))
;          (color (apply colorXRuleNode  checked?))
           (bordercolor (if checked? java.awt.Color.red nodeBorderColor))
          )
      (seq
        (setAttr node "xselect" (if checked? mode "none"))
;; should lookup color function in graph
        (invoke node "setFillColor" (apply colorXnetNode node)) 
        (invoke node "setBorderColor" bordercolor) 
        (invoke frame "repaint")
     )))  ; seq let lambda
)

(define  doXnetMouseClickedRuleAction (graph node e)  
  (let ((clist (object  ("java.util.ArrayList")))
        (frame (getAttr graph "frame"))
        (cbHideRule (object ("javax.swing.JCheckBox" "hideRule")))
        (acHideRule (apply mkXRuleCheckClosure frame node cbHideRule "t"))
        (selection (getAttr node "xselect" "none"))
      )
     (seq
      (invoke cbHideRule "setAction"
         (object ("g2d.closure.ClosureAbstractAction"
                  "hideRule" acHideRule)))
      (if (= selection "t")
          (invoke cbHideRule "setSelected" (boolean true))
       )
      (invoke clist "add" cbHideRule)
;      (invoke java.lang.System.err "println"  "rule calling extendSEMenu")
      (apply extendSEMenu graph clist)
     )
   )
)

(define mkXOccCheckClosure (frame node cb clist mode)
  (lambda (self e)
     (let ((checked? (invoke cb "isSelected"))
           (xstatus (getAttr node "xstatus" "none"))
           (bordercolor (if checked? java.awt.Color.red nodeBorderColor))
          )
      (seq
        (setAttr node "xselect" (if checked? mode "none"))
        (if checked?
           ; disable others
           (for cb1 clist 
             (if (not (= cb1 cb)) (invoke cb1 "setEnabled" (boolean false))) )
           ; enable all
           (for cb1 clist (invoke cb1 "setEnabled" (boolean true) ))
         )
        (invoke node "setFillColor" (apply colorXnetNode node)) 
        (invoke node "setBorderColor" bordercolor) 
        (invoke frame "repaint")
     )))  ; seq let lambda
)

(define  doXnetMouseClickedOccAction (graph node e)  
  (let ((clist (object ("java.util.ArrayList")))
        (frame (getAttr graph "frame"))
        (cbBoth (object ("javax.swing.JCheckBox" "explore Up+Down")))
        (acBoth (apply mkXOccCheckClosure frame node cbBoth clist "b"))
        (cbUp (object ("javax.swing.JCheckBox" "explore Up")))
        (acUp (apply mkXOccCheckClosure frame node cbUp clist "u"))
        (cbDown (object ("javax.swing.JCheckBox" "explore Down")))
        (acDown (apply mkXOccCheckClosure frame node cbDown clist "d"))
        (selection (getAttr node "xselect" "none"))
      )
     (seq
      (invoke cbBoth "setAction"
         (object ("g2d.closure.ClosureAbstractAction"
                  "explore Up&Down" acBoth)))
      (invoke cbUp "setAction"
         (object ("g2d.closure.ClosureAbstractAction"
                  "explore up stream" acUp)))
      (invoke cbDown "setAction"
         (object ("g2d.closure.ClosureAbstractAction"
                  "explore down stream" acDown)))
      (if (not (= selection "none"))
          (if (= selection "u")
              (invoke cbUp "setSelected" (boolean true))
          (if (= selection "d")
              (invoke cbDown "setSelected" (boolean true))
          (if (= selection "b")
              (invoke cbBoth "setSelected" (boolean true))
          ))) )
      (invoke clist "add" cbBoth)
      (invoke clist "add" cbUp)
      (invoke clist "add" cbDown)
;      (invoke java.lang.System.err "println"  "occ calling extendSEMenu")
      (apply extendSEMenu graph clist)
     )
   )
)

; for explore graph add checkBoxes to context menu tab
(define mkXnetMouseClickedClosure (graph)
  (lambda (self e)
    (seq 
;     (invoke java.lang.System.err "println" 
;          (concat e "\n" "xnet mouse click on " "\n" self))
      (if (instanceof self "pla.graph.PLANode")
        (let ((type (getAttr self "type")))
          (seq
          (if (= type "rule") 
            (apply doXnetMouseClickedRuleAction graph self e)             
          (if (= type "occ")
            (apply doXnetMouseClickedOccAction graph self e)
            (object null))) ; not a known node type
          ))
     (object null)  ; not a node
   ) ; if
  ) ;seq
 )
)

(define newNode (graph type mouseClickedClosure nid lab clab tags vals colorFun)
  (if (= type "occ")
    (apply newOccNode graph mouseClickedClosure nid lab clab tags vals colorFun)
    (if (= type "rule")
      (apply newRuleNode graph mouseClickedClosure nid lab clab tags vals colorFun)
      (object null)
    )) ; 2x if
)

(define newOccNode (graph mouseClickedClosure nid lab clab tags vals colorFun)
  (let ((label (if useChattyLabels
                   clab
                	(if (> (invoke lab "length") maxLabLen)
				          (invoke lab "substring" (int 0) maxLabLen) 
				          lab)
                   ))
        (node (object ("pla.graph.PLANode"
                       (object ("pla.data.Occurrence"
                                label
                                (sinvoke "java.lang.Integer" "parseInt" nid)
                                clab))
                       "ellipse"
                       nodeBorderColor
                       defaultFillColor)))
	 )
    (seq 
     (setAttr node "type" "occ")
     (setAttr node "nid" nid)
     (setAttr node "chattylabel" clab)
     (apply setAttrsAVX node tags vals)
     (invoke node "setFillColor" (apply colorFun node))
     (if (instanceof mouseClickedClosure "g2d.jlambda.Closure")
          (invoke node "setMouseAction"
                     java.awt.event.MouseEvent.MOUSE_CLICKED mouseClickedClosure)
      )
     (invoke graph "addNode" node)
     node))
) ;newOccNode

; add a rule node to given graph
(define newRuleNode (graph mouseClickedClosure nid lab clab tags vals colorFun)
  (let (
        (node (object ("pla.graph.PLANode"
		       (object ("pla.data.Rule" 
				lab 
				(sinvoke "java.lang.Integer" "parseInt" nid)
				clab))
		       "box"
		       nodeBorderColor
		       defaultFillColor)))
	)
    (seq 
     (setAttr node "type" "rule")
     (setAttr node "nid" nid)
     (setAttr node "chattylabel" clab)
     (apply setAttrsAVX node tags vals)
     (invoke node "setFillColor" (apply colorFun node))
     (if (instanceof mouseClickedClosure "g2d.jlambda.Closure")
         (invoke node "setMouseAction"
                     java.awt.event.MouseEvent.MOUSE_CLICKED mouseClickedClosure))
     (invoke graph "addNode" node) 
     node))
) ;addRuleNode


; add a node to given explore graph
(define newXNode (graph type mouseClickedClosure nid lab clab tags vals colorFun)
  (if (invoke graph "isDotLayout")
    (apply newNode graph type mouseClickedClosure nid lab clab tags vals colorFun)
    (let ((node (invoke graph "getPLANode" clab)))
      (if (instanceof node "pla.graph.PLANode")
        (seq 
          (apply setAttrsAVX node tags vals)
          (setAttr node "context" (object null))
          (invoke node "setFillColor" (apply colorFun node))
          (invoke node "setBorderColor" nodeBorderColor)
          (if (instanceof mouseClickedClosure "g2d.jlambda.Closure")
              (invoke node "setMouseAction"
                     java.awt.event.MouseEvent.MOUSE_CLICKED mouseClickedClosure))
          node
         )
        ;; shouldn't happen
          (apply newNode 
                 graph type mouseClickedClosure nid lab clab tags vals colorFun)
      ) ; if PLANode 
     ) ;let
  ) ; if isDot
)


(define updateXNode (graph nid xstatus colorFun)
  (let ((node (invoke graph "getNode" nid)))
  (seq
    (setAttr node "xselect" "none")
    (setAttr node "xstatus" xstatus)
    (invoke node "setFillColor" (apply colorFun node))
 ))
)        


(define newEdge (graph srcid tgtid bidir?)
  (let ((src (invoke graph "getNode" srcid))
        (tgt  (invoke graph "getNode" tgtid))
        (color (if (= bidir? "true") bidirEdgeColor  unidirEdgeColor))
        (e (object ("g2d.graph.IOPEdge" src tgt color))) )
    (seq 
      ; can replace "dashed" by "dotted"
     (if (= bidir? "true")  (invoke e "setStyle" "dashed"))
     (invoke e "setDoubleEnded" (boolean false))
     (invoke graph "addEdge" e)
     e))
) ;newEdge

(define newXEdge (graph srcid tgtid bidir?)
  (if (invoke graph "isDotLayout")
    (apply newEdge graph srcid tgtid bidir?)
    (let ((edge (invoke graph "getEdge"
                    (invoke graph "getNode" srcid)
                    (invoke graph "getNode" tgtid))
                 )
          )
      (if (instanceof edge "g2d.graph.IOPEdge")
          (seq (setAttr edge "context" (object null)) 
               (invoke edge "setColor" 
                      (if (= bidir? "true") bidirEdgeColor  unidirEdgeColor))
               edge)
          ;; shouldn't happen
          (apply newEdge graph srcid tgtid bidir?)
     ) ; if edge
   ) ; let
  ) ; if Dot
)


; remove a node from a given explore graph
(define delXNode (graph  nid)
  (let ((node  (invoke graph "getNode" nid)))
    (if (instanceof node "pla.graph.PLANode")
      (if (invoke graph "isDotLayout")
          (invoke graph "rmNode" node)
          (seq (setAttr node "context" "true")
               (setAttr node "xselect" "none")
               (invoke node "setFillColor" cxtFillColor)
               (invoke node "setBorderColor" cxtBorderColor)
               (invoke node "unsetMouseAction" 
                              java.awt.event.MouseEvent.MOUSE_CLICKED )
          )
      ) ; if isDot
    ) ; if PLANode 
  ) ;let
)

; cxtBorderColor

; remove a node from a given explore graph
(define delXEdge (graph  srcid tgtid)
  (let ((edge (invoke graph "getEdge"
                     (invoke graph "getNode" srcid)
                     (invoke graph "getNode" tgtid)) )
        )
    (if (instanceof edge "g2d.graph.IOPEdge")
      (if (invoke graph "isDotLayout")
          (invoke graph "rmEdge" edge)
          (seq (setAttr edge "context" "true")
               (invoke edge "setColor" cxtBorderColor))          
      ) ; if isDot
    ) ; if IOPEdge
  ) ;let
)



)
;;;;;;;;;; /Users/clt/bin/makeg2dlib loaded /Users/clt/Maude/Lib/M2.2/PLA1/G2dLib/coloring.lsp ;;;;;;;;;;
(seq

(define defaultFillColor java.awt.Color.white)

(define colorPnetNode (node)
  (let ((type (getAttr node "type" ""))
        (context? (getAttr node "context" "")) 
       )
    (if (= context? "true") cxtFillColor
    (if (= type "occ")    (apply colorPnetOccNode node)
    (if (= type "rule")    (apply colorPnetRuleNode node)
     java.awt.Color.white
    )))
  )
)


(define colorPnetOccNode (node)
  (let ((init (getAttr node "init" ""))
        (status (getAttr node "status" "none")) 
       )
    (if (= status "goal")  goalFillColor
    (if (= status "avoid") avoidFillColor
    (if (= init "true")  initFillColor
      noneFillColor
     )))  ; 5x if
    ) ; let
)
; trying coloring init with status
;    if (= status "none")  noneFillColor

(define colorPnetRuleNode (node)
  (let ((status (getAttr node "status" "none")))
    (if (= status "none") ruleFillColor
    (if (= status "avoid") avoidFillColor
      ruleFillColor
     ))  ; 2x if
    ) ; let
)

(define colorXnetNode (node)
   (let ((type (getAttr node "type" ""))
         (context? (getAttr node "context" "")) 
         )
    (if (= context? "true") cxtFillColor
    (if (= type "occ")    (apply colorXnetOccNode node)
    (if (= type "rule")    (apply colorXnetRuleNode node)
     java.awt.Color.white
    )))
  )
)


(define colorXnetRuleNode (node)
  (if (=  (getAttr node "xselect" "none") "none")
     ruleFillColor
     java.awt.Color.yellow )
)

(define colorXnetOccNode (node)
   (if (not (= (getAttr node "xselect" "none") "none"))
    ;; dispatch on selection mode
     java.awt.Color.yellow  
   ;; dispatch on xstatus
    (let ((xstatus (getAttr node "xstatus" "none")))
     (if (= xstatus "seen") java.awt.Color.lightGray
     (if (= xstatus "oup") initFillColor
     (if (= xstatus "odn") java.awt.Color.green
     (if (= xstatus "oboth") java.awt.Color.cyan 
      java.awt.Color.white
      )))) ; 4x if xstatus
     ) ;let
   )  ; if selected
)


(define colorCnetNode (node)
  (let ((compare (getAttr node "compare" "both"))
         (context? (getAttr node "context" "")) 
         )
    (if (= context? "true") cxtFillColor
    (if (= compare "left")  initFillColor  ; parent
    (if (= compare "right")
      (object ("java.awt.Color" (int 0) (int 255) (int 255)))
;;      java.awt.Color.white
      java.awt.Color.pink
     )) ) 
    ) ; let
)


(invoke java.lang.System.err "println"  "coloring.lsp loaded")
)
;;;;;;;;;; /Users/clt/bin/makeg2dlib loaded /Users/clt/Maude/Lib/M2.2/PLA1/G2dLib/newshow.lsp ;;;;;;;;;;
(seq

(define getKBMFrame ()
  (let ((kbm (fetch "KBManager")))
    (if (instanceof kbm "g2d.glyph.Attributable")
     (getAttr kbm "kbframe")
     (object null)
      ))
)

; (apply showNewGraph %gname %parent %selections %title %subtitle  %toolBarFun )
;; need to deal with menubar as well
;                      str  str or null bool                 id
(define showNewGraph (gname pname selections title subtitle toolBarFun)
  (let ((parent (if (instanceof pname "java.lang.String")
                 (fetch pname)
                 (object null)))
        (pframe (if (instanceof parent "pla.graph.PLAGraph")
                (getAttr parent "frame")
                (object null)))
        (frame (object ("pla.PLABaseFrame" "" 
            (if (instanceof pframe "pla.PLABaseFrame")  pframe (apply getKBMFrame))
             selections)) ) 
        (graph (fetch gname))
        (toolbar (invoke frame "getToolBar"))
	      )
    (seq
  ; prepend buttons and things in tool bar
  ; !!! need to deal with menus too
    (apply toolBarFun toolbar gname graph frame pname )
    (apply menuBarFunBase gname graph frame pname  )
    (apply anyShowGraph graph frame title subtitle)
  ))
)

(define menuBarFunBase (gname graph frame parent)
  (if (not (sinvoke "g2d.Main" "isRemote"))
    (apply menuBarFunBaseX gname graph frame parent)
  )
)

(define menuBarFunBaseX (gname graph frame parent)
  (let ((fileMenu (invoke frame "getFileMenu"))
        (menuItem (object ("javax.swing.JMenuItem"
                  "Export graph..." java.awt.event.KeyEvent.VK_G)))
        (toolkit  (sinvoke "java.awt.Toolkit" "getDefaultToolkit"))
        (keystrokeG (sinvoke "javax.swing.KeyStroke" "getKeyStroke"
                             java.awt.event.KeyEvent.VK_G 
                             (invoke toolkit "getMenuShortcutKeyMask") ))
        (clac
          (lambda (self event) 
            (let ((chooser (object ("g2d.swing.IOPFileChooser")))
                   (lspFilter (object ("g2d.swing.FileFilter" 
                              "JLambda *.lsp" "lsp")))
                   (pnFilter (object ("g2d.swing.FileFilter" 
                              "Petri net *.pn" "pn")))
                   (sbmlFilter (object ("g2d.swing.FileFilter" 
                              "Systems Biology Markup Language *.sbml" "sbml")))
                   )
              (seq
               (invoke chooser "setDialogTitle" "Export Graph To File")
               (invoke chooser "setAcceptAllFileFilterUsed" (boolean false))
               (invoke chooser "setMultiSelectionEnabled" (boolean false))
               (invoke chooser "addChoosableFileFilter" lspFilter)
               (invoke chooser "addChoosableFileFilter" pnFilter)
               (invoke chooser "addChoosableFileFilter" sbmlFilter)
               (invoke chooser "setFileFilter" lspFilter)
               (if (= (invoke chooser "showDialog" frame "Export") 
                       g2d.swing.IOPFileChooser.APPROVE_OPTION)
                 (let ((selectedFile (invoke chooser "getSelectedFile"))
                       (absPath (invoke selectedFile "getAbsolutePath"))
                       (canFileParent  (invoke (invoke selectedFile
                                       "getCanonicalFile") "getParent") )
                       (fileName (invoke selectedFile "getName") )
                      )
                  (seq
                   (invoke java.lang.System.err "println" 
                       (concat "Exporting " gname " to " absPath "\n"
                                 canFileParent "\n" fileName ))
                  (sinvoke "g2d.util.ActorMsg" "send" 
                      "maude" gname (concat "exportGraph" " " 
                                " " 
                         (apply fileNameExt fileName))  )
                   )) ; seq let
             ) ;if
             ) ;seq
           )) ;lambda
         ) ;clac
        (cla (object ("g2d.closure.ClosureActionListener"  clac )))
        )
;; (exportGraph graph2 graphics2d foo sbml)
    (seq
     (invoke menuItem "addActionListener" cla)
     (invoke menuItem "setAccelerator" keystrokeG)
     (invoke fileMenu "add" menuItem (int 3))
;    (invoke fileMenu "insertSeparator" (int 5))
    )
   )
)


 (define fileNameExt (str)
  (let ((ix (invoke str "lastIndexOf" "."))
        (base  (if (< ix (int 0))
                str
                (invoke str "substring" (int 0 ) ix )) )
        (ext  (if (< ix (int 0))
                "lsp"
                (invoke str "substring" (+ ix (int 1)))) )
       )
    (concat base " " ext)
   )
 )

(define toolBarFunBase (toolbar gname graph frame parent)
  (seq
;; incontext button if parent non null
     (if (not (= parent (object null)))
       (seq
         (invoke toolbar "prepend" 
             (sinvoke "pla.toolbar.SeparatorFactory" "makeLargeSep"))
         (invoke toolbar "prepend" (invoke frame "createLayoutButton" ))
      ))
;; add 2kb button
     (invoke toolbar "prepend" 
             (sinvoke "pla.toolbar.SeparatorFactory" "makeLargeSep"))
     (invoke toolbar "prepend" 
        (object ("pla.toolbar.ToolButton"
          (apply mkAction "ToKB" "Save underlying net as a KB" 
              (lambda (self event)
                (let ((ukbname (apply askUser 
                                     frame "AskUser" "Type in a KB name")))
                 (sinvoke "g2d.util.ActorMsg" "send" 
                   "maude" gname (concat "net2KB" " " ukbname))))
           ))))
;; add compare button
     (invoke toolbar "prepend" 
        (object ("pla.toolbar.ToolButton"
          (apply mkAction "Compare" "Compare this graph to another" 
              (lambda (self event)
                 (sinvoke "g2d.util.ActorMsg" "send" 
                   "maude" gname "compareGraphsChoose"))
           ))))
;; add compareMenu
     (invoke frame "addMenuAt" (invoke frame "getCompareMenu") (int 3))

  )
)

(define displaySEMenu (gname title clist)
  (let ((graph (fetch gname))
        (frame (getAttr graph "frame"))
        (gp (invoke frame "getGraphPanel"))
        (gpf (invoke gp "getFrame"))
        (sep (invoke gpf "getSEPanel"))
      )
   (invoke sep "displayMenu" title clist)
))

(define mkCompareSelectC (gname cgname)
   (lambda (self event)
     (sinvoke "g2d.util.ActorMsg" "send" 
       "maude" gname (concat "compareGraphs" " " cgname)))
)

(define compareSelect (gname choices)
  (let ((clist (object ("java.util.ArrayList"))))
    (seq 
     (for choice choices
       (if (> (lookup choice "length") (int 2) )
        (invoke clist "add"  
            (object ("g2d.swing.IOPButton" 
                 (concat (aget choice (int 0)) ": " (aget choice (int 1)))
                 (aget choice (int 2))     ; tip
                 (apply mkCompareSelectC gname (aget choice (int 0)))
            )))
        )) ; if for         
     (apply displaySEMenu gname "Select graph to compare" clist)
;  clist
   )) ; seq let
 )

(define pnetColorKey ()
  (let (
	     (colorkey (object ("g2d.swing.IOPColorKey")))
	     (colors (array java.awt.Color 
             initFillColor 
             noneFillColor
			    ruleFillColor
			    goalFillColor 
			    avoidFillColor 
			    cxtFillColor 
			    ))
	     (keys (array java.lang.String 
			  "Initial occurrence" 
			  "Occurrence no status" 
			  "Rule no status"
			  "Goal status"
			  "Avoid/hide status"
			  "Context node"
			  ))
			)
	 (seq 
	  (invoke colorkey "add" colors keys)
     colorkey
    )
  )
)

(define toolBarFunPnet (toolbar gname graph frame parent)
  (seq 
     (apply toolBarFunBase toolbar gname graph frame parent)
  ; prepend buttons and things in tool bar
     (invoke toolbar "add" (sinvoke "pla.toolbar.SeparatorFactory" "makeLargeSep"))
     (invoke toolbar "add" (apply pnetColorKey))
     (invoke toolbar "prepend" 
             (sinvoke "pla.toolbar.SeparatorFactory" "makeLargeSep"))
     (invoke toolbar "prepend" 
        (object ("pla.toolbar.ToolButton"
          (apply mkAction "FindPath" "find a path to goals" 
             (lambda (self event) (apply pathRequest graph))) ) ) )
     (invoke toolbar "prepend" 
        (object ("pla.toolbar.ToolButton"
          (apply mkAction "Subnet" "display relevant subnet" 
             (lambda (self event) (apply subnetRequest graph))) ) ) )
     (invoke toolbar "prepend" 
             (sinvoke "pla.toolbar.SeparatorFactory" "makeLargeSep"))
     (invoke toolbar "prepend" 
        (object ("pla.toolbar.ToolButton"
           (apply mkExploreInitAction gname "rule"
                  "Explore(Rules)"
                  "Explore starting from selected rules") )))
     (invoke toolbar "prepend" 
        (object ("pla.toolbar.ToolButton"
           (apply mkExploreInitAction gname "occ"
                  "Explore(Occs)"
                  "Explore starting from selected occurences") )))
  )
)


(define cnetColorKey ()
  (let (
	     (colorkey (object ("g2d.swing.IOPColorKey")))
	     (colors (array java.awt.Color 
             initFillColor
			    (object ("java.awt.Color" (int 0) (int 255) (int 255)))
			    java.awt.Color.pink
			    cxtFillColor 
			    ))
	     (keys (array java.lang.String 
			  "Requesting graph"
			  "CompareTo graph"
			  "Both graphs"
			  "context"
			  ))
			)
	 (seq 
	  (invoke colorkey "add" colors keys)
     colorkey
    )
  )
)



(define toolBarFunCnet (toolbar gname graph frame parent)
  (seq 
     (apply toolBarFunBase toolbar gname graph frame parent)
  ; prepend buttons and things in tool bar
     (invoke toolbar "add" (sinvoke "pla.toolbar.SeparatorFactory" "makeLargeSep"))
     (invoke toolbar "add" (apply cnetColorKey))
))

(define xnetColorKey ()
  (let (
	     (colorkey (object ("g2d.swing.IOPColorKey")))
	     (colors (array java.awt.Color 
             java.awt.Color.lightGray 
             initFillColor
			    java.awt.Color.green
			    java.awt.Color.cyan
			    java.awt.Color.yellow  
			    cxtFillColor 
			    ))
	     (keys (array java.lang.String 
			  "Occ node seen" 
			  "Occ node up OK"
			  "Occ node down OK"
			  "Occ node up and down OK"
			  "selected"
			  "context"
			  ))
			)
	 (seq 
	  (invoke colorkey "add" colors keys)
     colorkey
    )
  )
)



(define toolBarFunXnet (toolbar gname graph frame parent)
  (let ((cb (object ("pla.toolbar.ToolCheckBox" "New Frame")))
        (tf (object ("pla.toolbar.ToolTextField")))
        (gname (invoke graph "getUID"))
        (dnClosure 
           (lambda (self event)
             (let ((new (invoke cb "isSelected"))
                   (steps (invoke (invoke tf "getValue") "intValue"))
                  )
            (seq (sinvoke "g2d.util.ActorMsg" "send" "maude" gname 
                      (concat "explore " new " dn " steps))
                  (invoke cb "setSelected" (boolean false)))
            ))) ; dnClosure
        (downTip "Explore down given steps")
        (upClosure 
           (lambda (self event)
             (let ((new (invoke cb "isSelected"))
                   (steps (invoke (invoke tf "getValue") "intValue")))
             (sinvoke "g2d.util.ActorMsg" "send" "maude" gname 
                      (concat "explore " new " up " steps)))
            )) ; upClosure
        (upTip "Explore up given steps")
       ) ; letbindings
 ; prepend buttons and things in tool bar
    (seq
     (apply toolBarFunBase toolbar gname graph frame parent)
     (invoke toolbar "add" (sinvoke "pla.toolbar.SeparatorFactory" "makeLargeSep"))
     (invoke toolbar "add" (apply xnetColorKey))
     (invoke toolbar "prepend"
             (sinvoke "pla.toolbar.SeparatorFactory" "makeLargeSep"))
     ; check box for new or reuse frames
     (invoke cb "setToolTipText"
                "Open graph resulting from next explore operation in new frame")
     (invoke toolbar "prepend" cb)
     (invoke toolbar "prepend" 
             (sinvoke "pla.toolbar.SeparatorFactory" "makeSmallSep"))
     ; text field for number of steps
     (invoke tf "setToolTipText" 
                "Specify number of steps to be taken when exploring up or down")
     (invoke toolbar "prepend" tf)
     (invoke toolbar "prepend" 
                     (sinvoke "pla.toolbar.SeparatorFactory" "makeSmallSep"))
     ; up and down buttons
      (invoke toolbar "prepend"
        (object ("pla.toolbar.ToolButton" 
                 (apply mkAction "Down" downTip dnClosure))))
      (invoke toolbar "prepend"
        (object ("pla.toolbar.ToolButton" (apply mkAction "Up" upTip upClosure))))
     (invoke toolbar "prepend" 
        (object ("pla.toolbar.ToolButton"
          (apply mkAction "exploreSelected" "explore with xselect attributes" 
             (lambda (self event) (apply exploreSelectedRequest graph gname cb))) 
          ) ) ) 
   ; explore dropdown button
     (apply addExploreButton toolbar gname cb)
   ) ;seq
 ) ; let
) ; toolBarFunXnet


(define mkExploreClosure (gname cb cmd)
     (lambda (self event)
        (let ((new (invoke cb "isSelected")))
           (seq (sinvoke "g2d.util.ActorMsg" "send" "maude" gname 
                      (concat "explore " new " " cmd))
                (invoke cb "setSelected" (boolean false))
            )
         )
     ) ; exploreClosure
)


(define exploreSelectedRequest (graph gname cb)
   (let ((new (invoke cb "isSelected")))
     (seq (sinvoke "g2d.util.ActorMsg" "send" "maude" gname 
               (concat "explore " new " " "selected" " "
                       (apply mkXStatusString graph) ))
          (invoke cb "setSelected" (boolean false))
     ))
 ) 

(define mkXStatusString (graph)
  (let ((nodes (invoke graph "getNodesInArray")))
    (apply nodes2xselect nodes (int 0) (lookup nodes "length") "")
  ) 
)

(define nodes2xselect (nodes cur len str)
  (if (>= cur len)
   str
   (let ((node (aget nodes cur))
         (chatty (getAttr node "chattylabel" ""))
         (xselect (getAttr node "xselect" "none")))
    (apply nodes2xselect nodes (+ cur (int 1)) len 
         (if (or (= xselect "none") (= chatty ""))
          str 
          (concat str " " chatty " " xselect) 
      ) ) ) ; if app let 
  )  
) ; nodes2xselect


 
(invoke java.lang.System.err "println"  "newshow.lsp loaded")

)


)
