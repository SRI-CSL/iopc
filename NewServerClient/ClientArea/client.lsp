(let (
;      (host "10.3.0.152")
       (host "localhost")
;       (host (object null))
; dark's IP: 103.107.96.240
;      (host "dark.csl.sri.com")
      (port (int 7765))
      (socket  (object ("java.net.Socket" host port)))
      )
  (seq
   (load "pla.lsp")
   (sinvoke "g2d.Main" "main" socket)
   ) ; seq
  ) ; let
;; ii.maude needs server client versions of gname
;; server always graphics2d, client is remove if running remote client
