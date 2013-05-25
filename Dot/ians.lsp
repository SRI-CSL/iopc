
(define br2str (br)
  (let ((sb (object ("java.lang.StringBuffer"))))
    (seq 
     (apply br2str_aux br sb)
     (invoke sb "toString")
     )
    )
  )

(define br2str_aux (br sb)
  (let ((line (invoke br "readLine")))
    (if (isobject line)
        (seq
         (invoke sb "append" line)
         (invoke sb "append" "\n")
         (apply br2str_aux br sb)))))


(define url2str (urlstr)
  (let ((url (object ("java.net.URL" urlstr)))
        (connection (invoke url "openConnection"))
        (is (invoke connection "getInputStream"))
        (isr (object ("java.io.InputStreamReader" is)))
        (br (object ("java.io.BufferedReader" isr)))
        (str (apply br2str br)))
    (seq 
     (invoke br "close")
     str)
    )
  )
  
(define keggGlycanNumber2text (numberString)
  (let ((urlstr (concat "http://www.genome.jp/dbget-bin/www_bget?-f+k+glycan+" numberString)))
     (apply url2str urlstr)))
  
(define G00108 (apply keggGlycanNumber2text "G00108"))
(define G11334 (apply keggGlycanNumber2text "G11334"))




(define br2alist (br)
  (let ((alist (object ("java.util.ArrayList"))))
    (seq 
     (apply br2alist_aux br alist)
     alist
     )
    )
  )

(define br2alist_aux (br alist)
  (let ((line (invoke br "readLine")))
    (if (isobject line)
        (seq
         (invoke alist "add" line)
         (apply br2alist_aux br alist)))))

(define url2alist (urlstr)
  (let ((url (object ("java.net.URL" urlstr)))
        (connection (invoke url "openConnection"))
        (is (invoke connection "getInputStream"))
        (isr (object ("java.io.InputStreamReader" is)))
        (br (object ("java.io.BufferedReader" isr)))
        (alist (apply br2alist br)))
    (seq 
     (invoke br "close")
     alist)
    )
  )

(define keggGlycanNumber2alist (numberString)
  (let ((urlstr (concat "http://www.genome.jp/dbget-bin/www_bget?-f+k+glycan+" numberString)))
     (apply url2alist urlstr)))

(define AL_G00108 (apply keggGlycanNumber2alist "G00108"))
(define AL_G11334 (apply keggGlycanNumber2alist "G11334"))

(define printalist (alist) (for line alist (invoke java.lang.System.err "println" line)))
