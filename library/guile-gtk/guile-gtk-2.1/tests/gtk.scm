;; Copyright (C) 2006 Free Software Foundation, Inc.
;;
;; This program is free software; you can redistribute it and/or
;; modify it under the terms of the GNU General Public License as
;; published by the Free Software Foundation; either version 3 of
;; the License, or (at your option) any later version.
;; 
;; This program is distributed in the hope that it will be useful, but
;; WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;; General Public License for more details.
;; 
;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

(load "tests.scm")
(or (getenv "DISPLAY")
    (begin
      (display "Cannot run gtk tests without an X display\n")
      (exit 77)))

(use-modules (gtk-2.0 gtk)
	     (gtk-2.0 gdk))

;; the first-ever load of a given font name ties up some memory in a hash
;; table, or something, so make that happen here
(define sample-font     (gdk-font-load "fixed"))

;; don't put "(ash 1 256)" inside a `malloced-steady' because there was a
;; bug in guile 1.6.7 and earlier where the `bytes-malloced' wasn't
;; correctly maintained in bignum calculations
;;
(define sample-bignum (ash 1 256))

(define sample-unspecified (if #f #f))

(define sample-toplevel (gtk-window-new 'toplevel))
(gtk-widget-realize sample-toplevel) ;; realized but not mapped

;; guile 1.6 style make-shared-substring made available in guile 1.8
(if (not (defined? 'make-shared-substring))
    (define make-shared-substring substring/shared))

(define (exact-integer? x)
  (and (number? x)
       (integer? x)
       (exact? x)))

(define (exact-nonnegative-integer? x)
  (and (exact-integer? x)
       (>= x 0)))

(define (exact-positive-integer? x)
  (and (exact-integer? x)
       (positive? x)))


;;
;; gtk-adjustment-set-value
;;

(malloced-steady  ;; inum
 (lambda ()
   (let* ((adj   (gtk-adjustment-new 0 0 10 1 1 1))
	  (val   5)
	  (want  #t)
	  (got   (begin
		   (gtk-adjustment-set-value adj val)
		   #t)))
     (test "gtk-adjustment-set-value" val want got))))

(malloced-steady  ;; bignum
 (lambda ()
   (let* ((adj   (gtk-adjustment-new 0 0 10 1 1 1))
	  (val   sample-bignum)
	  (want  #t)
	  (got   (begin
		   (gtk-adjustment-set-value adj val)
		   #t)))
     (test "gtk-adjustment-set-value" val want got))))

(malloced-steady  ;; real
 (lambda ()
   (let* ((adj   (gtk-adjustment-new 0 0 10 1 1 1))
	  (val   5.0)
	  (want  #t)
	  (got   (begin
		   (gtk-adjustment-set-value adj val)
		   #t)))
     (test "gtk-adjustment-set-value" val want got))))

(malloced-steady  ;; complex
 (lambda ()
   (let* ((adj   (gtk-adjustment-new 0 0 10 1 1 1))
	  (val   5.0+5.0i)
	  (want  #f)
	  (got   (false-if-exception
		  (begin
		    (gtk-adjustment-set-value adj val)
		    #t))))
     (test "gtk-adjustment-set-value" val want got))))

;;
;; gtk-bin-child
;;

(malloced-steady
 (lambda ()
   (let* ((child (gtk-label-new "foo"))
	  (want  child)
	  (frame (gtk-frame-new "title"))
	  (got   (begin
		   (gtk-container-add frame child)
		   (gtk-bin-child frame))))
     (test "gtk-bin-child" #f want got))))

;;
;; gtk-bin-get-child
;;

(malloced-steady
 (lambda ()
   (let* ((child (gtk-label-new "foo"))
	  (want  child)
	  (frame (gtk-frame-new "title"))
	  (got   (begin
		   (gtk-container-add frame child)
		   (gtk-bin-get-child frame))))
     (test "gtk-bin-get-child" #f want got))))

;;
;; gtk-box-query-child-packing
;;

(malloced-steady
 (lambda ()
   (let* ((pack  '(#t #f 2 end))
	  (box   (gtk-hbox-new #f 0))
	  (child (gtk-label-new "foo"))
	  (want  pack)
	  (got   (begin
		   (gtk-container-add box child)
		   (apply gtk-box-set-child-packing box child pack)
		   (gtk-box-query-child-packing box child))))
     (test "gtk-box-query-child-packing" pack want got))))

;;
;; gtk-button-get-label
;;

(malloced-steady
 (lambda ()
   (let* ((button (gtk-button-new-with-label "foo"))
	  (want   "foo")
	  (got    (gtk-button-get-label button)))
     (test "gtk-button-get-label" button want got))))

;;
;; gtk-button-set-label
;;

(malloced-steady
 (lambda ()
   (let* ((button (gtk-button-new-with-label "foo"))
	  (want   "bar")
	  (got    (begin
		    (gtk-button-set-label button "bar")
		    (gtk-button-get-label button))))
     (test "gtk-button-get-label" button want got))))

;;
;; gtk-check-version
;;

(malloced-steady
 (lambda ()
   (let* ((args '(0 0 sample-bignum))
	  (want #f)
	  (got  (false-if-exception (apply gtk-check-version args))))
     (test "gtk-check-version" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args '(1 0 0))
	  (want #t)
	  (got  (string? (apply gtk-check-version args))))
     (test "gtk-check-version" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args (list gtk-major-version gtk-minor-version gtk-micro-version))
	  (want #f)
	  (got  (apply gtk-check-version args)))
     (test "gtk-check-version" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args '(999 2 0))
	  (want #t)
	  (got  (string? (apply gtk-check-version args))))
     (test "gtk-check-version" args want got))))

;;
;; gtk-clist-new-with-titles
;;

(malloced-steady  ;; list
 (lambda ()
   (let* ((title-list '("abc" "def"))
	  (clist      (gtk-clist-new-with-titles title-list))
	  (want       title-list)
	  (got        (list (gtk-clist-get-column-title clist 0)
			    (gtk-clist-get-column-title clist 1))))
     (test "gtk-clist-new-with-titles" title-list want got))))

(malloced-steady  ;; vector
 (lambda ()
   (let* ((title-vec (vector "abc" "def"))
	  (clist     (gtk-clist-new-with-titles title-vec))
	  (want      title-vec)
	  (got       (vector (gtk-clist-get-column-title clist 0)
			     (gtk-clist-get-column-title clist 1))))
     (test "gtk-clist-new-with-titles" title-vec want got))))

(malloced-steady  ;; number (bad)
 (lambda ()
   (let* ((title-list 999)
	  (want       #f)
	  (got        (false-if-exception
		       (gtk-clist-new-with-titles title-list))))
     (test "gtk-clist-new-with-titles" title-list want got))))

;;
;; gtk-combo-set-popdown-strings
;;

(define (my-gtk-list-item-string item)
  (gtk-label-get (gtk-bin-child item)))

(define (my-gtk-combo-get-popdown-strings combo)
  (map my-gtk-list-item-string
       (gtk-container-children (gtk-combo-list combo))))

(malloced-steady
 (lambda ()
   (let* ((strings '("foo"))
	  (want    strings)
	  (combo   (gtk-combo-new))
	  (got     (begin
		     (gtk-combo-set-popdown-strings combo strings)
		     (my-gtk-combo-get-popdown-strings combo))))
     ;; final free()s only take place under the main loop, so need this to
     ;; satisfy `malloced-steady
     (gtk-main-iteration-do #f)
     (test "gtk-combo-set-popdown-strings" strings want got))))

(malloced-steady
 (lambda ()
   (let* ((strings '("foo" "bar" "quux"))
	  (want    strings)
	  (combo   (gtk-combo-new))
	  (got     (begin
		     (gtk-combo-set-popdown-strings combo strings)
		     (my-gtk-combo-get-popdown-strings combo))))
     ;; final free()s only take place under the main loop, so need this to
     ;; satisfy `malloced-steady
     (gtk-main-iteration-do #f)
     (test "gtk-combo-set-popdown-strings" strings want got))))

(malloced-steady   ;; empty list is allowed
 (lambda ()
   (let* ((strings '())
	  (want    '())
	  (combo   (gtk-combo-new))
	  (got     (false-if-exception
		    (begin
		      (gtk-combo-set-popdown-strings combo strings)
		      (my-gtk-combo-get-popdown-strings combo)))))
     ;; final free()s only take place under the main loop, so need this to
     ;; satisfy `malloced-steady
     (gtk-main-iteration-do #f)
     (test "gtk-combo-set-popdown-strings" strings want got))))

(malloced-steady   ;; empty vector is allowed
 (lambda ()
   (let* ((strings (vector))
	  (want    '())
	  (combo   (gtk-combo-new))
	  (got     (false-if-exception
		    (begin
		      (gtk-combo-set-popdown-strings combo strings)
		      (my-gtk-combo-get-popdown-strings combo)))))
     ;; final free()s only take place under the main loop, so need this to
     ;; satisfy `malloced-steady
     (gtk-main-iteration-do #f)
     (test "gtk-combo-set-popdown-strings" strings want got))))

(malloced-steady   ;; bad strings
 (lambda ()
   (let* ((strings 12345)
	  (want    #f)
	  (combo   (gtk-combo-new))
	  (got     (false-if-exception
		    (begin
		      (gtk-combo-set-popdown-strings combo strings)
		      (my-gtk-combo-get-popdown-strings combo)))))
     ;; final free()s only take place under the main loop, so need this to
     ;; satisfy `malloced-steady
     (gtk-main-iteration-do #f)
     (test "gtk-combo-set-popdown-strings" strings want got))))

(malloced-steady   ;; non-string in list
 (lambda ()
   (let* ((strings '("foo" "bar" 12345))
	  (want    #f)
	  (combo   (gtk-combo-new))
	  (got     (false-if-exception
		    (begin
		      (gtk-combo-set-popdown-strings combo strings)
		      (my-gtk-combo-get-popdown-strings combo)))))
     ;; final free()s only take place under the main loop, so need this to
     ;; satisfy `malloced-steady
     (gtk-main-iteration-do #f)
     (test "gtk-combo-set-popdown-strings" strings want got))))

(malloced-steady   ;; non-string in vector
 (lambda ()
   (let* ((strings (vector "foo" "bar" 12345))
	  (want    #f)
	  (combo   (gtk-combo-new))
	  (got     (false-if-exception
		    (begin
		      (gtk-combo-set-popdown-strings combo strings)
		      (my-gtk-combo-get-popdown-strings combo)))))
     ;; final free()s only take place under the main loop, so need this to
     ;; satisfy `malloced-steady
     (gtk-main-iteration-do #f)
     (test "gtk-combo-set-popdown-strings" strings want got))))

;;
;; gtk-container-children
;;

(malloced-steady
 (lambda ()
   (let ((hbox (gtk-hbox-new #f 0)))
     (gtk-container-add hbox (gtk-label-new "one"))
     (gtk-container-add hbox (gtk-label-new "two"))
     (let* ((want 2)
	    (got  (length (gtk-container-children hbox))))
       (test "gtk-container-children" hbox want got)))))

;;
;; gtk-fixed-put
;;

(malloced-steady     ;; y too big
 (lambda ()
   (let* ((x     123)
	  (y     sample-bignum)
	  (fixed (gtk-fixed-new))
	  (child (gtk-label-new "foo"))
	  (want  #f)
	  (got   (false-if-exception
		  (begin
		    (gtk-fixed-put fixed child x y)
		    #t))))
     (test "gtk-fixed-put" (list x y) want got))))

(malloced-steady
 (lambda ()
   (let* ((x     123)
	  (y     456)
	  (fixed (gtk-fixed-new))
	  (child (gtk-label-new "foo"))
	  (want  #t)
	  (got   (false-if-exception
		  (begin
		    (gtk-fixed-put fixed child x y)
		    #t))))
     (test "gtk-fixed-put" (list x y) want got))))

;;
;; gtk-item-factory-path-from-widget
;;

(malloced-steady
 (lambda ()
   (let* ((widget (gtk-label-new "foo"))
	  (want   #f)
	  (got    (gtk-item-factory-path-from-widget widget)))
     (test "gtk-item-factory-path-from-widget" widget want got))))

;; not wrapped yet
;;
;; (malloced-steady
;;  (lambda ()
;;    (let* ((factory (gtk-item-factory-new 'container "foo" #f))
;;           ...
;; 	  (widget  (gtk-item-factory-get-widget factory "bar"))
;; 	  (want    factory)
;; 	  (got     (gtk-item-factory-path-from-widget widget)))
;;      (test "gtk-rc-get-default-files" widget want got))))

;;
;; gtk-rc-get-default-files
;;

(malloced-steady
 (lambda ()
   (let* ((want #t)
	  (lst  (gtk-rc-get-default-files))
	  (got  (and (>= (length lst) 1)
		     (every string? (gtk-rc-get-default-files)))))
     (test "gtk-rc-get-default-files" #f want got))))

;;
;; gtk-object-get
;;

(malloced-steady
 (lambda ()
   (let* ((label (gtk-label-new "some text")))

     (let* ((prop #:label)
	    (want "some text")
	    (got  (gtk-object-get label prop)))
       (test "gtk-object-get label" prop want got)))))

(malloced-steady
 (lambda ()
   (let* ((label (gtk-label-new "some text")))

     (let* ((prop 'label)
	    (want "some text")
	    (got  (gtk-object-get label prop)))
       (test "gtk-object-get label" prop want got)))))

(malloced-steady
 (lambda ()
   (let* ((label (gtk-label-new "some text")))

     (let* ((prop 12345)
	    (want #f)
	    (got  (false-if-exception (gtk-object-get label prop))))
       (test "gtk-object-get label" prop want got)))))

;;
;; gtk-label-new
;;

(malloced-steady  ;; string
 (lambda ()
   (let* ((str  "foo")
	  (want #t)
	  (got  (->bool (gtk-label-new str))))
     (test "gtk-label-new" str want got))))

(malloced-steady  ;; empty string
 (lambda ()
   (let* ((str  "")
	  (want #t)
	  (got  (->bool (gtk-label-new str))))
     (test "gtk-label-new" str want got))))

(malloced-steady  ;; #f
 (lambda ()
   (let* ((str  #f)
	  (want #t)
	  (got  (->bool (gtk-label-new str))))
     (test "gtk-label-new" str want got))))

(malloced-steady  ;; bad
 (lambda ()
   (let* ((str  1234)
	  (want #f)
	  (got  (false-if-exception (->bool (gtk-label-new str)))))
     (test "gtk-label-new" str want got))))

;;
;; gtk-label-set-text
;;

(malloced-steady  ;; string
 (lambda ()
   (let* ((label (gtk-label-new #f))
	  (str   "foo")
	  (want  #t)
	  (got   (->bool (gtk-label-set-text label str))))
     (test "gtk-label-set-text" str want got))))

(malloced-steady  ;; empty string
 (lambda ()
   (let* ((label (gtk-label-new #f))
	  (str   "")
	  (want  #t)
	  (got   (->bool (gtk-label-set-text label str))))
     (test "gtk-label-set-text" str want got))))

(malloced-steady  ;; #f is bad
 (lambda ()
   (let* ((label (gtk-label-new #f))
	  (str   #f)
	  (want  #f)
	  (got   (false-if-exception (->bool (gtk-label-set-text label str)))))
     (test "gtk-label-set-text" str want got))))

(malloced-steady  ;; bad
 (lambda ()
   (let* ((label (gtk-label-new #f))
	  (str   1234)
	  (want  #f)
	  (got   (false-if-exception (->bool (gtk-label-set-text label str)))))
     (test "gtk-label-set-text" str want got))))

;;
;; gtk-major-version
;;

(malloced-steady
 (lambda ()
   (let* ((want #t)
	  (got  (and (exact-nonnegative-integer? gtk-major-version)
		     (>= gtk-major-version 2))))
     (test "gtk-major-version" #f want got))))

;;
;; gtk-minor-version
;;

(malloced-steady
 (lambda ()
   (let* ((want #t)
	  (got  (exact-nonnegative-integer? gtk-minor-version)))
     (test "gtk-minor-version" #f want got))))

;;
;; gtk-micro-version
;;

(malloced-steady
 (lambda ()
   (let* ((want #t)
	  (got  (exact-nonnegative-integer? gtk-micro-version)))
     (test "gtk-micro-version" #f want got))))

;;
;; gtk-binary-age
;;

(malloced-steady
 (lambda ()
   (let* ((want #t)
	  (got  (exact-nonnegative-integer? gtk-binary-age)))
     (test "gtk-binary-age" #f want got))))

;;
;; gtk-idle-remove
;;

(malloced-steady
 (lambda ()
   (let* ((id   (gtk-idle-add noop))
	  (want #t)
	  (got  (begin
		  (gtk-idle-remove id)
		  #t)))
     (test "gtk-idle-remove" id want got))))

;;
;; gtk-input-remove
;;

;; port buffer space miscounted in malloc stats in guile 1.6, so don't have
;; the pipe inside malloced-steady
;;
(let ((port  (cdr (pipe))))
  (malloced-steady
   (lambda ()
     (let* ((id   (gtk-input-add port '(write) noop))
	    (want #t)
	    (got  (begin
		    (gtk-input-remove id)
		    #t)))
       (test "gtk-input-remove" id want got)))))

;;
;; gtk-interface-age
;;

(malloced-steady
 (lambda ()
   (let* ((want #t)
	  (got  (exact-nonnegative-integer? gtk-interface-age)))
     (test "gtk-interface-age" #f want got))))

;;
;; gtk-object-new
;;

(malloced-steady
 (lambda ()
   (let* ((args  (list 'GtkLabel))
	  (want  #t)
	  (got   (gtk-label? (apply gtk-object-new args))))
     (test "gtk-object-new" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args (list 'NoSuchType))
	  (want #f)
	  (got  (false-if-exception (apply gtk-object-new args))))
     (test "gtk-object-new" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args  (list 'GtkObject))  ;; cannot be instantiated
	  (want  #f)
	  (got   (false-if-exception (apply gtk-object-new args))))
     (test "gtk-object-new" args want got))))

;;
;; gtk-object-new label
;;

(malloced-steady
 (lambda ()
   (let* ((args  (list 'GtkLabel #:label "foo"))
	  (want  "foo")
	  (label (apply gtk-object-new args))
	  (got   (gtk-object-get label #:label)))
     (test "gtk-object-new label" args want got))))

;;
;; gtk-item-factory-path-from-widget
;;

(malloced-steady
 (lambda ()
   (let* ((widget (gtk-label-new "foo"))
	  (want   #f)
	  (got    (gtk-item-factory-path-from-widget widget)))
     (test "gtk-item-factory-path-from-widget" widget want got))))

;;
;; gtk-object-set string
;;

(malloced-steady
 (lambda ()
   (let* ((label (gtk-label-new #f))
	  (str   "foo")
	  (want  str)
	  (got   (begin
		   (gtk-object-set label #:label str)
		   (gtk-object-get label #:label))))
     (test "gtk-object-set string" str want got))))

;;
;; gtk-paint-hline
;;


(malloced-steady
 (lambda ()
   (let* ((args (list (gtk-widget-get-style sample-toplevel)
		      (gtk-widget-window sample-toplevel)
		      'normal
		      #f
		      sample-toplevel
		      #f
		      10 20 10))
	  (want #t)
	  (got  (begin
		  (apply gtk-paint-hline args)
		  #t)))
     (test "gtk-paint-hline" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args (list (gtk-widget-get-style sample-toplevel)
		      (gtk-widget-window sample-toplevel)
		      'normal
		      '((0 . 0) . (100 . 200))
		      sample-toplevel
		      #f
		      10 20 10))
	  (want #t)
	  (got  (begin
		  (apply gtk-paint-hline args)
		  #t)))
     (test "gtk-paint-hline" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args (list (gtk-widget-get-style sample-toplevel)
		      (gtk-widget-window sample-toplevel)
		      'normal
		      '((0 . 0) . (100 . 200))
		      sample-toplevel
		      "label"
		      10 20 10))
	  (want #t)
	  (got  (begin
		  (apply gtk-paint-hline args)
		  #t)))
     (test "gtk-paint-hline" args want got))))

;;
;; gtk-radio-button-group
;;

(malloced-steady
 (lambda ()
   (let* ((rbutton (gtk-radio-button-new-from-widget #f))
	  (want    (list rbutton))
	  (got     (gtk-radio-button-group rbutton)))
     (test "gtk-radio-button-group" rbutton want got))))


;;
;; gtk-scrolled-window-new
;;

(malloced-steady
 (lambda ()
   (let* ((args       '())
	  (scrolled   (apply gtk-scrolled-window-new args))
	  (want       #t)
	  (got        (and (gtk-adjustment?
			    (gtk-scrolled-window-get-hadjustment scrolled))
			   (gtk-adjustment?
			    (gtk-scrolled-window-get-vadjustment scrolled)))))
     (test "gtk-scrolled-window-new" args want got))))

(malloced-steady
 (lambda ()
   (let* ((hadj       (gtk-adjustment-new 0 0 1 0.5 0.5 0.5))
	  (args       (list hadj))
	  (scrolled   (apply gtk-scrolled-window-new args))
	  (want       #t)
	  (got        (and (eq? hadj
				(gtk-scrolled-window-get-hadjustment scrolled))
			   (gtk-adjustment?
			    (gtk-scrolled-window-get-vadjustment scrolled)))))
     (test "gtk-scrolled-window-new" args want got))))

(malloced-steady
 (lambda ()
   (let* ((hadj       (gtk-adjustment-new 0 0 1 0.5 0.5 0.5))
	  (vadj       (gtk-adjustment-new 0 0 1 0.5 0.5 0.5))
	  (args       (list hadj vadj))
	  (scrolled   (apply gtk-scrolled-window-new args))
	  (want       args)
	  (got        (list (gtk-scrolled-window-get-hadjustment scrolled)
			    (gtk-scrolled-window-get-vadjustment scrolled))))
     (test "gtk-scrolled-window-new" args want got))))

(malloced-steady
 (lambda ()
   (let* ((vadj       (gtk-adjustment-new 0 0 1 0.5 0.5 0.5))
	  (args       (list #f vadj))
	  (scrolled   (apply gtk-scrolled-window-new args))
	  (want       #t)
	  (got        (and (gtk-adjustment?
			    (gtk-scrolled-window-get-hadjustment scrolled))
			   (eq? vadj
				(gtk-scrolled-window-get-vadjustment scrolled)))))
     (test "gtk-scrolled-window-new" args want got))))

;;
;; gtk-signal-disconnect
;;

(malloced-steady
 (lambda ()
   (let* ((button (gtk-button-new))
	  (id     (gtk-signal-connect button "clicked" noop)))
     (gtk-signal-disconnect button id)
     ;; only checking for no memory leak
     (test "gtk-signal-disconnect" button #t #t))))

;;
;; gtk-timeout-remove
;;

(malloced-steady
 (lambda ()
   (let* ((id   (gtk-timeout-add 100000 noop))
	  (want #t)
	  (got  (begin
		  (gtk-timeout-remove id)
		  #t)))
     (test "gtk-timeout-remove" id want got))))


;;
;; gtk-toolbar-set-style
;;

(malloced-steady
 (lambda ()
   (let* ((toolbar (gtk-toolbar-new))
	  (style   'both)
	  (want    style)
	  (got     (begin
		     (gtk-toolbar-set-style toolbar style)
		     (gtk-toolbar-get-style toolbar))))
     (test "gtk-toolbar-set-style" (list toolbar style) want got))))

;;
;; gtk-widget-size-request string
;;

(malloced-steady
 (lambda ()
   (let* ((label (gtk-label-new "foo"))
	  (want  #t)
	  (pair  (gtk-widget-size-request label))
	  (got   (and (pair? pair)
		      (exact-positive-integer? (car pair))
		      (exact-positive-integer? (cdr pair)))))
     (test "gtk-widget-size-request" (list label pair) want got))))

;;
;; signal call font
;;

(gtk-signal-new-generic "test-call-font" '() 'GtkLabel 'void '(GdkFont))
(let ((obj (gtk-object-new 'GtkLabel))
      (ret #f))
  (gtk-signal-connect obj "test-call-font" noop)

  (malloced-steady  ;; font
   (lambda ()
     (let* ((font  sample-font)
	    (want  #t)
	    (got   (begin
		     (gtk-signal-emit obj "test-call-font" font)
		     #t)))
       (test "gtk-signal-emit call font" font want got))))

  (malloced-steady  ;; string
   (lambda ()
     (let* ((font  "fixed")
	    (want  #t)
	    (got   (begin
		     (gtk-signal-emit obj "test-call-font" font)
		     #t)))
       (test "gtk-signal-emit call font" font want got)))))

;;
;; signal call string+int
;;

(gtk-signal-new-generic "test-call-string+int" '() 'GtkLabel
			'void '(gchararray gint))
(let ((obj (gtk-object-new 'GtkLabel))
      (ret #f))
  (gtk-signal-connect obj "test-call-string+int" noop)

  (malloced-steady  ;; good
   (lambda ()
     (let* ((args  (list "foo" 123))
	    (want  #t)
	    (got   (begin
		     (apply gtk-signal-emit obj "test-call-string+int" args)
		     #t)))
       (test "gtk-signal-emit call string+int" args want got))))

  (malloced-steady  ;; int too big
   (lambda ()
     (let* ((args  (list "foo" sample-bignum))
	    (want  #f)
	    (got   (false-if-exception
		    (begin
		      (apply gtk-signal-emit obj "test-call-string+int" args)
		      #t))))
       (test "gtk-signal-emit call string+int" args want got)))))

;;
;; signal call return string
;;

;; Somehow the return is always GTK_TYPE_NONE so you don't get back a
;; string.  The tests here are in readiness if that's fixed.

(gtk-signal-new-generic "test-call-return-string" '() 'GtkLabel
			'gchararray '())
(let ((obj (gtk-object-new 'GtkLabel))
      (ret #f))

  (gtk-signal-connect obj "test-call-return-string"
    (lambda ()
      ret))

  (malloced-steady
   (lambda ()
     (let* ((str  "")
	    (want #t)
	    (got  (begin
		    (set! ret str)
		    (gtk-signal-emit obj "test-call-return-string")
		    #t)))
       (test "gtk-signal-emit call return string" str want got))))

  (malloced-steady
   (lambda ()
     (let* ((str  "foo")
	    (want #t)
	    (got  (begin
		    (set! ret str)
		    (gtk-signal-emit obj "test-call-return-string")
		    #t)))
       (test "gtk-signal-emit call return string" str want got))))

  (malloced-steady
   (lambda ()
     (let* ((str  (make-shared-substring (string #\nul #\x #\nul) 1 2))
	    (want #t)
	    (got  (begin
		    (set! ret str)
		    (gtk-signal-emit obj "test-call-return-string")
		    #t)))
       (test "gtk-signal-emit call return string" str want got))))

;; FIXME: Don't want to print error message.

;;   (malloced-steady  ;; bad type
;;    (lambda ()
;;      (let* ((str  999)
;; 	    (want #t)
;; 	    (got  (begin
;; 		    (set! ret str)
;; 		    (gtk-signal-emit obj "test-call-return-string")
;; 		    #t)))
;;        (test "gtk-signal-emit call return string" str want got))))

;;   (malloced-steady  ;; bomb with \0 in string
;;    (lambda ()
;;      (let* ((str  (string #\nul))
;; 	    (want #t)
;; 	    (got  (begin
;; 		    (set! ret str)
;; 		    (gtk-signal-emit obj "test-call-return-string")
;; 		    #t)))
;;        (test "gtk-signal-emit call return string" str want got))))
  )

;;
;; signal call GType (new in glib 2.12)
;;

(gtk-signal-new-generic "test-call-gtype" '() 'GtkLabel 'void '(GType))
(let ((obj          (gtk-object-new 'GtkLabel))
      (arg-received #f))
  (gtk-signal-connect obj "test-call-gtype"
    (lambda (arg)
      (set! arg-received arg)))

  (malloced-steady  ;; gtype
   (lambda ()
     (let* ((type (gtk-type-from-name "GdkWindow"))
	    (want type)
	    (got  (begin
		    (gtk-signal-emit obj "test-call-gtype" type)
		    arg-received)))
       (test "gtk-signal-emit call gtype" type want got))))

  (malloced-steady  ;; symbol in
   (lambda ()
     (let* ((type 'GdkWindow)
	    (want (gtk-type-from-name (symbol->string type)))
	    (got  (begin
		    (gtk-signal-emit obj "test-call-gtype" type)
		    arg-received)))
       (test "gtk-signal-emit call gtype" type want got))))

  (malloced-steady  ;; number - bogus
   (lambda ()
     (let* ((type 999)
	    (want #f)
	    (got  (false-if-exception
		   (begin
		     (gtk-signal-emit obj "test-call-gtype" type)
		     #t))))
       (test "gtk-signal-emit call gtype" type want got)))))


(tests-end)
