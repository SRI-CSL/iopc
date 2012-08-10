;(sinvoke "PLA3" "main" (object null))

(seq 
 (sinvoke "TabUtils" "launchTab" (object null) "Titanic" "Will she float or will she sink")

 (let ((main (sinvoke "TabUtils" "launchTab" (object null) "Graph 1" "Main Dish"))
       (child1 (sinvoke "TabUtils" "launchTab" main "Graph 2" "Subnet of 1"))
       (child2 (sinvoke "TabUtils" "launchTab" child1 "Graph 3" "Findpath of 2"))
       (child3 (sinvoke "TabUtils" "launchTab" main "Graph 4" "Another subnet of 1"))
       (child4 (sinvoke "TabUtils" "launchTab" main "Graph 5" "Compare of 1 and 3"))
       )

   (invoke java.lang.System.err "println" "Boo!"))
)

