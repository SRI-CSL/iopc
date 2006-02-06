(seq
 (load "~/IOP/Client/graph.lsp")
 (define host "localhost")
 (define port (int 8764))

 (let (
       (socket  (object ("java.net.Socket" host port)))
       (main    (lambda (self) (sinvoke "g2d.Main" "main" socket)))
       (thread  (object ("g2d.closure.ClosureThread" main)))
       )
   (seq
    (invoke thread "start")
    (apply displayGraph)
    ) ; seq
   ) ; let
 )
