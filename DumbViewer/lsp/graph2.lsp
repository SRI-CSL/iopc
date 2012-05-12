(seq 

;;   (load "../../PLALib/g2dlib.lsp")
   (define useChattyLabels (boolean true))
;;   (load "dumb.lsp")
   (apply protoKBM "TKB")

   (let ((makeGraph (lambda (graph mouseClickedClosure)
        (seq
         (apply newOccNode graph mouseClickedClosure "20003" "Tgfb1" "Tgfb1-XOut" (array java.lang.String  "init" "status") (array java.lang.String  "true" "none") colorPnetNode)
(apply newOccNode graph mouseClickedClosure "20004" "TgfbR1" "TgfbR1-Tgfb1RC" (array java.lang.String  "init" "status") (array java.lang.String  "true" "none") colorPnetNode)
(apply newOccNode graph mouseClickedClosure "20005" "TgfbR2" "TgfbR2-Tgfb1RC" (array java.lang.String  "init" "status") (array java.lang.String  "true" "none") colorPnetNode)
(apply newOccNode graph mouseClickedClosure "1927" "Tgfb1:TgfbR1-act:TgfbR2-act" "Tgfb1:TgfbR1-act:TgfbR2-act-Tgfb1RC" (array java.lang.String  "init" "status") (array java.lang.String  "" "none") colorPnetNode)
(apply newRuleNode graph mouseClickedClosure "20006" "931" "931.TgfbR1.TgfbR2.by.Tgfb1" (array java.lang.String  "status" "pre" "post") (array java.lang.String  "none" "20003 20004 20005" "1927") colorPnetNode)
(apply newOccNode graph mouseClickedClosure "2197" "Smad4" "Smad4-CLc" (array java.lang.String  "init" "status") (array java.lang.String  "true" "none") colorPnetNode)
(apply newOccNode graph mouseClickedClosure "2195" "Smad4" "Smad4-NUc" (array java.lang.String  "init" "status") (array java.lang.String  "" "none") colorPnetNode)
(apply newRuleNode graph mouseClickedClosure "11033" "768" "768.Smad4.to.nuc.irt.Tgfb1" (array java.lang.String  "status" "pre" "post") (array java.lang.String  "none" "2197 1927" "2195 1927") colorPnetNode)
(apply newOccNode graph mouseClickedClosure "20757" "Smad4-sumo" "Smad4-sumo-NUc" (array java.lang.String  "init" "status") (array java.lang.String  "" "goal") colorPnetNode)
(apply newOccNode graph mouseClickedClosure "20758" "Pias2" "Pias2-NUc" (array java.lang.String  "init" "status") (array java.lang.String  "true" "none") colorPnetNode)
(apply newOccNode graph mouseClickedClosure "20759" "Sae1" "Sae1-NUc" (array java.lang.String  "init" "status") (array java.lang.String  "true" "none") colorPnetNode)
(apply newOccNode graph mouseClickedClosure "20760" "Uba2" "Uba2-NUc" (array java.lang.String  "init" "status") (array java.lang.String  "true" "none") colorPnetNode)
(apply newOccNode graph mouseClickedClosure "20761" "Ube2i" "Ube2i-NUc" (array java.lang.String  "init" "status") (array java.lang.String  "true" "none") colorPnetNode)
(apply newRuleNode graph mouseClickedClosure "20762" "853c" "853c.Smad4.by.Ube2i" (array java.lang.String  "status" "pre" "post") (array java.lang.String  "none" "2195 20758 20759 20760 20761" "20757 20758 20759 20760 20761") colorPnetNode)
(apply newOccNode graph mouseClickedClosure "20763" "Pias4" "Pias4-NUc" (array java.lang.String  "init" "status") (array java.lang.String  "true" "none") colorPnetNode)
(apply newRuleNode graph mouseClickedClosure "20764" "853c-1" "853c.Smad4.by.Ube2i#1" (array java.lang.String  "status" "pre" "post") (array java.lang.String  "none" "2195 20763 20759 20760 20761" "20757 20763 20759 20760 20761") colorPnetNode)

         (apply newEdge graph "20003" "20006" "false")
(apply newEdge graph "20004" "20006" "false")
(apply newEdge graph "20005" "20006" "false")
(apply newEdge graph "20006" "1927" "false")
(apply newEdge graph "2197" "11033" "false")
(apply newEdge graph "11033" "2195" "false")
(apply newEdge graph "1927" "11033" "true")
(apply newEdge graph "2195" "20762" "false")
(apply newEdge graph "20762" "20757" "false")
(apply newEdge graph "20758" "20762" "true")
(apply newEdge graph "20759" "20762" "true")
(apply newEdge graph "20760" "20762" "true")
(apply newEdge graph "20761" "20762" "true")
(apply newEdge graph "2195" "20764" "false")
(apply newEdge graph "20764" "20757" "false")
(apply newEdge graph "20763" "20764" "true")
(apply newEdge graph "20759" "20764" "true")
(apply newEdge graph "20760" "20764" "true")
(apply newEdge graph "20761" "20764" "true")

      )))
   (graph (object ("g2d.graph.IOPGraph")))
;;; !!! NEW
   (mouseClickedClosure (apply mkDumbMouseClickedClosure graph) )
 ) ;; let
   (seq
     (apply makeGraph graph mouseClickedClosure)
     (invoke graph "setUID" "graph2")
     (apply setAttrsAV graph (array java.lang.String  "kbname" "unRchGoals" "source" "subtitle") (array java.lang.String  "TKB" "" "pnet pnet3" " Goals: Smad4-sumo-NUc 
") (int 4) (int 0))
     (apply setAttrsAV graph
          (array java.lang.String  "requestor" "requestop" "requestargs")
          (array java.lang.String  "TKB" "dishnet" "tgfb1-smad4-suomo-nuc") 
     (int 3) (int 0))
     (setAttr graph "colorFun" colorPnetNode)
     graph
   )
 ) ;; let
;;; !!! NEW
   (apply showDumbGraph "graph2")
;;   (object null) (boolean true))
;; "Subnet of Tgfb1Dish" " Goals: Smad4-sumo-NUc "  toolBarFunPnet 
 )  ;; topseq
