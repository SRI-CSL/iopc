(let (
     (host "10.3.0.50")
;      (host "localhost")
;      (host "dark.csl.sri.com")
;      (port (int 8765))
      (port (int 7765))
      (socket  (object ("java.net.Socket" host port)))
      )
  (seq
   (load "g2dlib.lsp")
   (sinvoke "g2d.Main" "main" socket)
   ) ; seq
  ) ; let
;; ii.maude needs server client versions of gname
;; server always graphics2d, client is remove if running remote client
