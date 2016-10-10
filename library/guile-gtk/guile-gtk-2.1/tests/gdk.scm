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
      (display "Cannot run gdk tests without an X display\n")
      (exit 77)))

(use-modules (srfi srfi-1)
	     (gtk-2.0 gdk)
	     (gtk-2.0 gtk))

(define sample-win     (gdk-window-new #f 100 100 '() 'input-output 'toplevel))
(define sample-gc       (gdk-gc-new sample-win))
(define sample-visual   (gdk-window-get-visual sample-win))
(define sample-colormap (gdk-window-get-colormap sample-win))
;; the first-ever load of a given font name ties up some memory in a hash
;; table, or something, so make that happen here
(define sample-font     (gdk-font-load "fixed"))
(define sample-bitmap   (gdk-pixmap-new #f 1 1 1))

;; don't put "(ash 1 256)" inside a `malloced-steady' because there was a
;; bug in guile 1.6.7 and earlier where the `bytes-malloced' wasn't
;; correctly maintained in bignum calculations
;;
(define sample-bignum (ash 1 256))

(if (not (defined? 'make-typed-array))
    (define (make-typed-array type fill . bounds)  ;; for 1.6
      (case type
	((s8 u8)
	 (let ((array (apply make-uniform-array #\nul bounds)))
	   (array-fill! array fill)
	   array))
	(else
	 (error "make-typed-array: unrecognised type:" type)))))

;; guile 1.6 style make-shared-substring made available in guile 1.8
(if (not (defined? 'make-shared-substring))
    (define make-shared-substring substring/shared))  ;; for 1.6

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
;; gdk-drawable-get-image
;;

(malloced-steady
 (lambda ()
   (let* ((image (gdk-drawable-get-image sample-bitmap 0 0 1 1))
	  (want  #t)
	  ;; just let malloced-steady see our ref counting on the returned
	  ;; image is right
	  (got   (gdk-image? image)))
     (test "gdk-drawable-get-image" image want got))))

;;
;; gdk-drawable-copy-to-image
;;

(malloced-steady
 (lambda ()
   (let* ((image (gdk-drawable-copy-to-image sample-bitmap #f 0 0 0 0 1 1))
	  (want  #t)
	  ;; just let malloced-steady see our ref counting on the returned
	  ;; image is right
	  (got   (gdk-image? image)))
     (test "gdk-drawable-copy-to-image, new" image want got))))

(malloced-steady
 (lambda ()
   (let* ((orig-image (gdk-drawable-get-image sample-bitmap 0 0 1 1))
	  (new-image  (gdk-drawable-copy-to-image sample-bitmap orig-image
						  0 0 0 0 1 1))
	  (want       #t)
	  ;; just let malloced-steady see our ref counting on the returned
	  ;; image is right
	  (got        (gdk-image? new-image)))
     (test "gdk-drawable-copy-to-image, existing" orig-image want got))))

;;
;; GdkFunction
;;

(let* ((bitmap (gdk-pixmap-new #f 1 1 1))
       (gc0    (gdk-gc-new bitmap))
       (gc1    (gdk-gc-new bitmap))
       (gc     (gdk-gc-new bitmap))
       (color  (gdk-color-new)))

  (define (my-gdk-get-point drawable x y)
    (let ((image (gdk-drawable-get-image drawable x y 1 1)))
      (gdk-image-get-pixel image 0 0)))

  (define (attempt function src dst want)

    (gdk-color-set-pixel color src)
    (gdk-gc-set-foreground gc color)
    (gdk-gc-set-function gc function)

    (gdk-draw-point bitmap (if (= 0 dst) gc0 gc1) 0 0)

    (gdk-draw-point bitmap gc 0 0)

    (malloced-steady
     (lambda ()
       (let ((got (my-gdk-get-point bitmap 0 0)))
	 (test "GdkFunction using gdk-draw-point" (list function src dst)
	       want got)))))

  (gdk-color-set-pixel color 0)
  (gdk-gc-set-foreground gc0 color)
  (gdk-gc-set-function gc0 'copy)

  (gdk-color-set-pixel color 1)
  (gdk-gc-set-foreground gc1 color)
  (gdk-gc-set-function gc1 'copy)

  (attempt 'copy 0 0 0)
  (attempt 'copy 0 1 0)
  (attempt 'copy 1 0 1)
  (attempt 'copy 1 1 1)

  (attempt 'invert 0 0 1)
  (attempt 'invert 0 1 0)
  (attempt 'invert 1 0 1)
  (attempt 'invert 1 1 0)

  (attempt 'xor 0 0 0)
  (attempt 'xor 0 1 1)
  (attempt 'xor 1 0 1)
  (attempt 'xor 1 1 0)

  (attempt 'clear 0 0 0)
  (attempt 'clear 0 1 0)
  (attempt 'clear 1 0 0)
  (attempt 'clear 1 1 0)

  (attempt 'and 0 0 0)
  (attempt 'and 0 1 0)
  (attempt 'and 1 0 0)
  (attempt 'and 1 1 1)

  (attempt 'and-reverse 0 0 0)
  (attempt 'and-reverse 0 1 0)
  (attempt 'and-reverse 1 0 1)
  (attempt 'and-reverse 1 1 0)

  (attempt 'and-invert 0 0 0)
  (attempt 'and-invert 0 1 1)
  (attempt 'and-invert 1 0 0)
  (attempt 'and-invert 1 1 0)

  (attempt 'noop 0 0 0)
  (attempt 'noop 0 1 1)
  (attempt 'noop 1 0 0)
  (attempt 'noop 1 1 1)

  (attempt 'or 0 0 0)
  (attempt 'or 0 1 1)
  (attempt 'or 1 0 1)
  (attempt 'or 1 1 1)

  (attempt 'equiv 0 0 1)
  (attempt 'equiv 0 1 0)
  (attempt 'equiv 1 0 0)
  (attempt 'equiv 1 1 1)

  (attempt 'or-reverse 0 0 1)
  (attempt 'or-reverse 0 1 0)
  (attempt 'or-reverse 1 0 1)
  (attempt 'or-reverse 1 1 1)

  (attempt 'copy-invert 0 0 1)
  (attempt 'copy-invert 0 1 1)
  (attempt 'copy-invert 1 0 0)
  (attempt 'copy-invert 1 1 0)

  (attempt 'or-invert 0 0 1)
  (attempt 'or-invert 0 1 1)
  (attempt 'or-invert 1 0 0)
  (attempt 'or-invert 1 1 1)

  (attempt 'nand 0 0 1)
  (attempt 'nand 0 1 1)
  (attempt 'nand 1 0 1)
  (attempt 'nand 1 1 0)

  (attempt 'nor 0 0 1)
  (attempt 'nor 0 1 0)
  (attempt 'nor 1 0 0)
  (attempt 'nor 1 1 0)

  (attempt 'set 0 0 1)
  (attempt 'set 0 1 1)
  (attempt 'set 1 0 1)
  (attempt 'set 1 1 1))

;;
;; gdk-char-width
;;

(malloced-steady
 (lambda ()
   (let* ((char  #\m)
	  (want  #t)
	  (got   (number? (gdk-char-width sample-font char))))
     (test "gdk-char-width" char want got))))

(malloced-steady
 (lambda ()
   (let* ((char  "m")  ;; bad
	  (want  #f)
	  (got   (false-if-exception
		  (begin
		    (gdk-char-width sample-font char)
		    #t))))
     (test "gdk-char-width" char want got))))

(malloced-steady
 (lambda ()
   (let* ((char  65)  ;; bad
	  (want  #f)
	  (got   (false-if-exception
		  (begin
		    (gdk-char-width sample-font char)
		    #t))))
     (test "gdk-char-width" char want got))))

;;
;; gdk-atom-intern
;;

(malloced-steady
 (lambda ()
   (let* ((name   9999)  ;; bad arg
	  (notnew #f)
	  (want   #f)
	  (got    (false-if-exception (gdk-atom-intern name notnew))))
     (test "gdk-atom-intern" (list name notnew) want got))))

(malloced-steady
 (lambda ()
   (let* ((name   'not-a-string)  ;; bad arg
	  (notnew #f)
	  (want   #f)
	  (got    (false-if-exception (gdk-atom-intern name notnew))))
     (test "gdk-atom-intern" (list name notnew) want got))))

;; Gtk 2.0 ignores the notnew flag, atom always created.
;;
;; (malloced-steady
;;  (lambda ()
;;    (let* ((name   "ATOM_NOT_OTHERWISE_EXISTING")
;; 	  (notnew #t)
;; 	  (want   #f)
;; 	  (got    (gdk-atom-intern name notnew)))
;;      (test "gdk-atom-intern" (list name notnew) want got))))

(malloced-steady
 (lambda ()
   (let* ((name   "FOO_BAR_QUUX")
	  (notnew #f)
	  (want   'FOO_BAR_QUUX)
	  (got    (gdk-atom-intern name notnew)))
     (test "gdk-atom-intern" (list name notnew) want got))))

;;
;; gdk-atom-name
;;

(malloced-steady
 (lambda ()
   (let* ((atom   'WM_NAME)
	  (want   "WM_NAME")
	  (got    (gdk-atom-name atom)))
     (test "gdk-atom-name" atom want got))))

(malloced-steady
 (lambda ()
   (let* ((atom   'BLAH_BLAH_BLAH)
	  (want   "BLAH_BLAH_BLAH")
	  (got    (gdk-atom-name atom)))
     (test "gdk-atom-name" atom want got))))

;;
;; gdk-bitmap-create-from-data
;;

(malloced-steady
 (lambda ()
   (let* ((data   (string (integer->char #x00) (integer->char #x00)
			  (integer->char #x0F) (integer->char #x0F)))
	  (bitmap (gdk-bitmap-create-from-data #f data 4 4))
	  (image  (gdk-image-get bitmap 0 0 4 4))
	  (want   '(0 0 1 1))
	  (got    (list (gdk-image-get-pixel image 0 0)
			(gdk-image-get-pixel image 0 1)
			(gdk-image-get-pixel image 0 2)
			(gdk-image-get-pixel image 0 3))))
     (test "gdk-bitmap-create-from-data" data want got))))

(malloced-steady
 (lambda ()
   (let* ((data   (vector #x00 #x00 #x0F #x0F))
	  (bitmap (gdk-bitmap-create-from-data #f data 4 4))
	  (image  (gdk-image-get bitmap 0 0 4 4))
	  (want   '(0 0 1 1))
	  (got    (list (gdk-image-get-pixel image 0 0)
			(gdk-image-get-pixel image 0 1)
			(gdk-image-get-pixel image 0 2)
			(gdk-image-get-pixel image 0 3))))
     (test "gdk-bitmap-create-from-data" data want got))))

(malloced-steady
 (lambda ()
   (let* ((data   (make-typed-array 's8 0 4))
	  (bitmap (begin
		    (array-set! data #x00 0)
		    (array-set! data #x00 1)
		    (array-set! data #x0F 2)
		    (array-set! data #x0F 3)
		    (gdk-bitmap-create-from-data #f data 4 4)))
	  (image  (gdk-image-get bitmap 0 0 4 4))
	  (want   '(0 0 1 1))
	  (got    (list (gdk-image-get-pixel image 0 0)
			(gdk-image-get-pixel image 0 1)
			(gdk-image-get-pixel image 0 2)
			(gdk-image-get-pixel image 0 3))))
     (test "gdk-bitmap-create-from-data" data want got))))

(malloced-steady
 (lambda ()
   (let* ((data   (make-typed-array 'u8 0 4))
	  (bitmap (begin
		    (array-set! data #x00 0)
		    (array-set! data #x00 1)
		    (array-set! data #x0F 2)
		    (array-set! data #x0F 3)
		    (gdk-bitmap-create-from-data #f data 4 4)))
	  (image  (gdk-image-get bitmap 0 0 4 4))
	  (want   '(0 0 1 1))
	  (got    (list (gdk-image-get-pixel image 0 0)
			(gdk-image-get-pixel image 0 1)
			(gdk-image-get-pixel image 0 2)
			(gdk-image-get-pixel image 0 3))))
     (test "gdk-bitmap-create-from-data" data want got))))

;; this provokes a "deprecated" warning from guile 1.8 but we still want to
;; exercise this old style, for now
(malloced-steady
 (lambda ()
   (let* ((data   (make-uniform-array #\nul 4))  ;; old Guile 1.6 style
	  (bitmap (begin
		    (array-set! data #x00 0)
		    (array-set! data #x00 1)
		    (array-set! data #x0F 2)
		    (array-set! data #x0F 3)
		    (gdk-bitmap-create-from-data #f data 4 4)))
	  (image  (gdk-image-get bitmap 0 0 4 4))
	  (want   '(0 0 1 1))
	  (got    (list (gdk-image-get-pixel image 0 0)
			(gdk-image-get-pixel image 0 1)
			(gdk-image-get-pixel image 0 2)
			(gdk-image-get-pixel image 0 3))))
     (test "gdk-bitmap-create-from-data" data want got))))

;;
;; gdk-char-width
;;

(malloced-steady
 (lambda ()
   (let* ((char  #\m)
	  (want  #t)
	  (got   (number? (gdk-char-width sample-font char))))
     (test "gdk-char-width" char want got))))

(malloced-steady
 (lambda ()
   (let* ((char  "m")  ;; bad
	  (want  #f)
	  (got   (false-if-exception
		  (begin
		    (gdk-char-width sample-font char)
		    #t))))
     (test "gdk-char-width" char want got))))

(malloced-steady
 (lambda ()
   (let* ((char  65)  ;; bad
	  (want  #f)
	  (got   (false-if-exception
		  (begin
		    (gdk-char-width sample-font char)
		    #t))))
     (test "gdk-char-width" char want got))))

;;
;; gdk-color-black
;;

(malloced-steady
 (lambda ()
   (let* ((color (gdk-color-black sample-colormap))
	  (want  #t)
	  (got   (or (eq? #f color)
		     (eqv? 0 (gdk-color-red   color))
		     (eqv? 0 (gdk-color-green color))
		     (eqv? 0 (gdk-color-blue  color)))))
     (test "gdk-color-black" color want got))))

;;
;; gdk-color-white
;;

(malloced-steady
 (lambda ()
   (let* ((color (gdk-color-white sample-colormap))
	  (want  #t)
	  (got   (or (eq? #f color)
		     (eqv? 65535 (gdk-color-red   color))
		     (eqv? 65535 (gdk-color-green color))
		     (eqv? 65535 (gdk-color-blue  color)))))
     (test "gdk-color-white" color want got))))

;;
;; gdk-colormap-alloc-colors
;;

(let ((visual (gdk-visual-get-best-with-type 'pseudo-color)))
  (if visual
      (begin
	(malloced-steady
	 (lambda ()
	   (let* ((colormap (gdk-colormap-new visual #f))
		  (colors   (vector "black" "white"))
		  (want     #t)
		  (got      (list? (gdk-colormap-alloc-colors
				    colormap colors #f #t))))
	     (test "gdk-colormap-alloc-colors" colors want got))))

	(malloced-steady
	 (lambda ()
	   (let* ((colormap (gdk-colormap-new visual #f))
		  (colors   (vector "black" "nosuchcolourname"))
		  (want     #f)
		  (got      (false-if-exception
			     (begin
			       (gdk-colormap-alloc-colors colormap
							  colors #f #t)
			       #t))))
	     (test "gdk-colormap-alloc-colors" colors want got))))

	(malloced-steady
	 (lambda ()
	   (let* ((colormap (gdk-colormap-new visual #f))
		  (colors   (vector "black" (string #\nul)))
		  (want     #f)
		  (got      (false-if-exception
			     (begin
			       (gdk-colormap-alloc-colors colormap
							  colors #f #t)
			       #t))))
	     (test "gdk-colormap-alloc-colors" colors want got)))))

      (display "skip gdk-colormap-alloc-colors: no pseudo-color visual\n")))

;;
;; gdk-colormap-query-color
;;

(malloced-steady
 (lambda ()
   (let* ((white       (gdk-color-white sample-colormap))
	  (white-pixel (gdk-color-pixel white))
	  (result      (gdk-color-new))
	  (want        #t)
	  (got (begin
		 (gdk-colormap-query-color sample-colormap white-pixel
					   result)
		 (and
		  (eqv? (gdk-color-red   white) (gdk-color-red   result))
		  (eqv? (gdk-color-blue  white) (gdk-color-blue  result))
		  (eqv? (gdk-color-green white) (gdk-color-green result))))))
     (test "gdk-colormap-query-color" white want got))))

;;
;; gdk-drag-begin
;;

(malloced-steady
 (lambda ()
   (let* ((targets  '())
	  (want   #t)
	  (got    (begin
		    (gdk-drag-begin sample-win targets)
		    #t)))
     (test "gdk-drag-begin" targets want got))))

(malloced-steady
 (lambda ()
   (let* ((targets  '(STRING))
	  (want   #t)
	  (got    (begin
		    (gdk-drag-begin sample-win targets)
		    #t)))
     (test "gdk-drag-begin" targets want got))))

;;
;; gdk-draw-text
;;

(malloced-steady
 (lambda ()
   (let* ((args   (list sample-win sample-font sample-gc 10 10 "hello"))
	  (want   #t)
	  (got    (begin
		    (apply gdk-draw-text args)
		    #t)))
     (test "gdk-draw-text" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args   (list sample-win sample-font sample-gc 10 10
			(string #\nul #\x #\nul)))
	  (want   #t)
	  (got    (begin
		    (apply gdk-draw-text args)
		    #t)))
     (test "gdk-draw-text" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args   (list sample-win sample-font sample-gc 10 10
			(make-shared-substring (string #\nul #\x #\nul) 1 2)))
	  (want   #t)
	  (got    (begin
		    (apply gdk-draw-text args)
		    #t)))
     (test "gdk-draw-text" args want got))))

;;
;; gdk-event-string
;;

(if (getenv "GUILE_GTK_TESTS_INTERACTIVE")
    (let ((press    #f)
	  (release  #f)
	  (toplevel (gtk-window-new 'toplevel)))
      (gtk-widget-add-events toplevel '(key-press-mask key-release-mask))
      (gtk-signal-connect toplevel "key_press_event"
	(lambda (event)
	  (set! press event)))
      (gtk-signal-connect toplevel "key_release_event"
	(lambda (event)
	  (set! release event)))
      (gtk-widget-show-all toplevel)

      (display "Please press a key ... ")
      (force-output)
      (gtk-widget-show-all toplevel)
      (while (not (and press release))
	(gtk-main-iteration-do #f))
      (display "thanks.\n")
      (force-output)
      (gtk-widget-destroy toplevel)

      (malloced-steady
       (lambda ()
	 (let* ((event press)
		(str   (gdk-event-string event))
		(want  #t)
		(got   (string? str)))
	   (test "gdk-event-string" event want got))))

      ;; release event is usually NULL, allow for that
      (malloced-steady
       (lambda ()
	 (let* ((event release)
		(str   (gdk-event-string event))
		(want  #t)
		(got   (or (string? str)
			   (not str))))
	   (test "gdk-event-string" event want got))))))

;;
;; gdk-font?
;;

(malloced-steady
 (lambda ()
   (let* ((obj  sample-font)
	  (want #t)
	  (got  (gdk-font? obj)))
     (test "gdk-font?" obj want got))))

(malloced-steady
 (lambda ()
   (let* ((obj  123)
	  (want #f)
	  (got  (gdk-font? obj)))
     (test "gdk-font?" obj want got))))

(malloced-steady
 (lambda ()
   (let* ((obj  sample-win)
	  (want #f)
	  (got  (gdk-font? obj)))
     (test "gdk-font?" obj want got))))

(malloced-steady
 (lambda ()
   (let* ((obj  "fixed")
	  (want #f)
	  (got  (gdk-font? obj)))
     (test "gdk-font?" obj want got))))

;;
;; gdk-font-id
;;

(malloced-steady
 (lambda ()
   (let* ((font sample-font)
	  (want #t)
	  (got  (exact-integer? (gdk-font-id font))))
     (test "gdk-font-load" font want got))))

(malloced-steady
 (lambda ()
   (let* ((font "fixed")
	  (want #t)
	  (got  (exact-integer? (gdk-font-id font))))
     (test "gdk-font-load" font want got))))

;;
;; gdk-font-load
;;

(malloced-steady
 (lambda ()
   (let* ((name "fixed")
	  (want #t)
	  (got  (->bool (gdk-font-load name))))
     (test "gdk-font-load" name want got))))

(malloced-steady
 (lambda ()
   (let* ((name "fixedXX")
	  (want #f)
	  (got  (->bool (gdk-font-load name))))
     (test "gdk-font-load" name want got))))

(malloced-steady
 (lambda ()
   (let* ((name (make-shared-substring "fixedXX" 0 5))
	  (want #t)
	  (got  (->bool (gdk-font-load name))))
     (test "gdk-font-load" name want got))))

;;
;; gdk-gc-new
;;

(malloced-steady
 (lambda ()
   (let* ((want #t)
	  (got  (->bool (gdk-gc-new sample-win))))
     (test "gdk-gc-new" sample-win want got))))

(malloced-steady
 (lambda ()
   (let* ((pixmap (gdk-pixmap-new #f 1 1 8))
	  (want   #t)
	  (got    (->bool (gdk-gc-new pixmap))))
     (test "gdk-gc-new" pixmap want got))))

;;
;; gdk-gc-new-with-values
;;

(malloced-steady
 (lambda ()
   (let* ((args (list sample-win #:bogus))
	  (want #f)
	  (got  (false-if-exception (apply gdk-gc-new-with-values args))))
     (test "gdk-gc-new-with-values" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args (list sample-win #:font "fixed"))
	  (want #t)
	  (got  (->bool (apply gdk-gc-new-with-values args))))
     (test "gdk-gc-new-with-values" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args (list sample-win #:font "fixed"))
	  (want #t)
	  (got  (->bool (apply gdk-gc-new-with-values args))))
     (test "gdk-gc-new-with-values" args want got))))


;;
;; gdk-gc-set-dashes
;;

(malloced-steady
 (lambda ()
   (let* ((args  '(5 (6 7 8)))
	  (want  #t)
	  (got   (begin
		   (apply gdk-gc-set-dashes sample-gc args)
		   #t)))
     (test "gdk-gc-set-dashes" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args  '(x (6 7 8)))  ;; bad dash_offset
	  (want  #f)
	  (got   (false-if-exception
		  (begin
		    (apply gdk-gc-set-dashes sample-gc args)
		    #t))))
     (test "gdk-gc-set-dashes" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args  '(5 (x 1)))
	  (want  #f)
	  (got   (false-if-exception
		  (begin
		    (apply gdk-gc-set-dashes sample-gc args)
		    #t))))
     (test "gdk-gc-set-dashes" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args  '(5 (1 x)))
	  (want  #f)
	  (got   (false-if-exception
		  (begin
		    (apply gdk-gc-set-dashes sample-gc args)
		    #t))))
     (test "gdk-gc-set-dashes" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args  '(5 (-128 127)))
	  (want  #t)
	  (got   (begin
		   (apply gdk-gc-set-dashes sample-gc args)
		   #t)))
     (test "gdk-gc-set-dashes" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args  '(5 (1 128))) ;; too big
	  (want  #f)
	  (got   (false-if-exception
		  (begin
		    (apply gdk-gc-set-dashes sample-gc args)
		    #t))))
     (test "gdk-gc-set-dashes" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args  '(5 (1 -129))) ;; too big negative
	  (want  #f)
	  (got   (false-if-exception
		  (begin
		    (apply gdk-gc-set-dashes sample-gc args)
		    #t))))
     (test "gdk-gc-set-dashes" args want got))))

;;
;; gdk-gc-set-foreground
;;

(malloced-steady
 (lambda ()
   (let* ((color "black")
	  (want  #t)
	  (got   (begin
		   (gdk-gc-set-foreground sample-gc color)
		   #t)))
     (test "gdk-gc-set-foreground" color want got))))

(malloced-steady
 (lambda ()
   (let* ((color "blackXXX")
	  (want  #f)
	  (got   (false-if-exception
		  (begin
		    (gdk-gc-set-foreground sample-gc color)
		    #t))))
     (test "gdk-gc-set-foreground" color want got))))

(malloced-steady
 (lambda ()
   (let* ((color (make-shared-substring "blackXXX" 0 5))
	  (want  #t)
	  (got   (begin
		   (gdk-gc-set-foreground sample-gc color)
		   #t)))
     (test "gdk-gc-set-foreground" color want got))))

;;
;; gdk-get-leader-window-id
;;

(malloced-steady
 (lambda ()
   (let* ((id   (gdk-get-leader-window-id))
	  (want #t)
	  (got  (exact-positive-integer? id)))
     (test "gdk-get-leader-window-id" #f want got))))

;;
;; gdk-image-new
;;

(malloced-steady
 (lambda ()
   (let* ((args  (list 'fastest (gdk-visual-get-system) 10 10))
	  (image (apply gdk-image-new args)))
     ;; only checking no memory leak
     (test "gdk-image-new" args #t #t))))

;;
;; gdk-image-new-bitmap
;;

(malloced-steady
 (lambda ()
   (let* ((data  (string (integer->char #x7F) (integer->char #x80)
			 (integer->char #x7F) (integer->char #x80)))
	  (image (gdk-image-new-bitmap (gdk-visual-get-system) data 9 2))
	  (want  '(0 1 0 1))
	  (got   (list (gdk-image-get-pixel image 0 0)
		       (gdk-image-get-pixel image 1 0)
		       (gdk-image-get-pixel image 0 1)
		       (gdk-image-get-pixel image 1 1))))
     (test "gdk-image-new-bitmap" data want got))))

;;
;; gdk-major-version
;;

(malloced-steady
 (lambda ()
   (let* ((want #t)
	  (got  (and (exact-nonnegative-integer? gdk-major-version)
		     (>= gdk-major-version 2))))
     (test "gdk-major-version" gdk-major-version want got))))

;;
;; gdk-minor-version
;;

(malloced-steady
 (lambda ()
   (let* ((want #t)
	  (got  (exact-nonnegative-integer? gdk-minor-version)))
     (test "gdk-minor-version" gdk-minor-version want got))))

;;
;; gdk-pixmap-new
;;

(malloced-steady
 (lambda ()
   (let* ((args   '(#f 1 1 8))
	  (want   #t)
	  (got    (->bool (apply gdk-pixmap-new args))))
     (test "gdk-pixmap-new" args want got))))

;;
;; gdk-priority-events
;;

(malloced-steady
 (lambda ()
   (let* ((want #t)
	  (got  (exact-integer? gdk-priority-events)))
     (test "gdk-priority-events" gdk-priority-events want got))))

;;
;; gdk-property-change
;;

(malloced-steady  ;; null '()
 (lambda ()
   (let* ((args   (list sample-win 'SOME-PROPERTY 'STRING 8 'replace
			'()))
	  (want   '("" STRING 8))
	  (got    (begin
		    (apply gdk-property-change args)
		    (gdk-property-get sample-win 'SOME-PROPERTY 'STRING 0 100 #f))))
     (test "gdk-property-change" args want got))))

(malloced-steady  ;; string
 (lambda ()
   (let* ((args   (list sample-win 'SOME-PROPERTY 'STRING 8 'replace "hello"))
	  (want   '("hello" STRING 8))
	  (got    (begin
		    (apply gdk-property-change args)
		    (gdk-property-get sample-win 'SOME-PROPERTY 'STRING 0 100 #f))))
     (test "gdk-property-change" args want got))))

(malloced-steady  ;; list
 (lambda ()
   (let* ((args   (list sample-win 'SOME-PROPERTY 'STRING 8 'replace
			'(65 66 67)))
	  (want   (list "ABC" 'STRING 8))
	  (got    (begin
		    (apply gdk-property-change args)
		    (gdk-property-get sample-win 'SOME-PROPERTY 'STRING 0 100 #f))))
     (test "gdk-property-change" args want got))))

(malloced-steady  ;; vector
 (lambda ()
   (let* ((args   (list sample-win 'SOME-PROPERTY 'STRING 8 'replace
			(vector 67 66 65)))
	  (want   '("CBA" STRING 8))
	  (got    (begin
		    (apply gdk-property-change args)
		    (gdk-property-get sample-win 'SOME-PROPERTY 'STRING 0 100 #f))))
     (test "gdk-property-change" args want got))))

(malloced-steady  ;; vector 8 with bad value
 (lambda ()
   (let* ((args   (list sample-win 'SOME-PROPERTY 'STRING 8 'replace
			(vector 67 'x 65)))
	  (want   #f)
	  (got    (false-if-exception
		   (begin
		     (apply gdk-property-change args)
		     #t))))
     (test "gdk-property-change" args want got))))

(malloced-steady  ;; vector 8 with neg value
 (lambda ()
   (let* ((args   (list sample-win 'SOME-PROPERTY 'STRING 8 'replace
			(vector 67 -1 65)))
	  (want   #f)
	  (got    (false-if-exception
		   (begin
		     (apply gdk-property-change args)
		     #t))))
     (test "gdk-property-change" args want got))))

(malloced-steady  ;; vector 8 with too-big value
 (lambda ()
   (let* ((args   (list sample-win 'SOME-PROPERTY 'STRING 8 'replace
			(vector 256 sample-bignum 65)))
	  (want   #f)
	  (got    (false-if-exception
		   (begin
		     (apply gdk-property-change args)
		     #t))))
     (test "gdk-property-change" args want got))))

(malloced-steady  ;; vector 16 with bad value
 (lambda ()
   (let* ((args   (list sample-win 'SOME-PROPERTY 'STRING 16 'replace
			(vector 67 'x 65)))
	  (want   #f)
	  (got    (false-if-exception
		   (begin
		     (apply gdk-property-change args)
		     #t))))
     (test "gdk-property-change" args want got))))

(malloced-steady  ;; vector 16 with neg value
 (lambda ()
   (let* ((args   (list sample-win 'SOME-PROPERTY 'STRING 16 'replace
			(vector 67 -1 65)))
	  (want   #f)
	  (got    (false-if-exception
		   (begin
		     (apply gdk-property-change args)
		     #t))))
     (test "gdk-property-change" args want got))))

(malloced-steady  ;; vector 16 with too-big value
 (lambda ()
   (let* ((args   (list sample-win 'SOME-PROPERTY 'STRING 16 'replace
			(vector 67 65536 65)))
	  (want   #f)
	  (got    (false-if-exception
		   (begin
		     (apply gdk-property-change args)
		     #t))))
     (test "gdk-property-change" args want got))))

(malloced-steady  ;; vector 32 with bad value
 (lambda ()
   (let* ((args   (list sample-win 'SOME-PROPERTY 'STRING 32 'replace
			(vector 67 'x 65)))
	  (want   #f)
	  (got    (false-if-exception
		   (begin
		     (apply gdk-property-change args)
		     #t))))
     (test "gdk-property-change" args want got))))

(malloced-steady  ;; vector 32 with neg value
 (lambda ()
   (let* ((args   (list sample-win 'SOME-PROPERTY 'STRING 32 'replace
			(vector 67 -1 65)))
	  (want   #f)
	  (got    (false-if-exception
		   (begin
		     (apply gdk-property-change args)
		     #t))))
     (test "gdk-property-change" args want got))))

(malloced-steady  ;; vector 32 with too-big value
 (lambda ()
   (let* ((args   (list sample-win 'SOME-PROPERTY 'STRING 32 'replace
			(vector 67 4294967296 65)))
	  (want   #f)
	  (got    (false-if-exception
		   (begin
		     (apply gdk-property-change args)
		     #t))))
     (test "gdk-property-change" args want got))))

;;
;; gdk-query-depths
;;

(malloced-steady
 (lambda ()
   (let* ((want   #t)
	  (got    (every integer? (gdk-query-depths))))
     (test "gdk-query-depths" #f want got))))

;;
;; gdk-screen-width
;;

(malloced-steady
 (lambda ()
   (let* ((want #t)
	  (got  (exact-positive-integer? (gdk-screen-width))))
     (test "gdk-screen-width" #f want got))))

;;
;; gdk-screen-height
;;

(malloced-steady
 (lambda ()
   (let* ((want #t)
	  (got  (exact-positive-integer? (gdk-screen-height))))
     (test "gdk-screen-height" #f want got))))

;;
;; gdk-screen-width-mm
;;

(malloced-steady
 (lambda ()
   (let* ((want #t)
	  (got  (exact-positive-integer? (gdk-screen-width-mm))))
     (test "gdk-screen-width-mm" #f want got))))

;;
;; gdk-screen-height-mm
;;

(malloced-steady
 (lambda ()
   (let* ((want #t)
	  (got  (exact-positive-integer? (gdk-screen-height-mm))))
     (test "gdk-screen-height-mm" #f want got))))

;;
;; gdk-selection-property-get
;;

(malloced-steady  ;; with nothing ever requested
 (lambda ()
   (let* ((win    sample-win)
	  (want   '(#f NONE 0))
	  (got    (gdk-selection-property-get win)))
     (test "gdk-selection-property-get" sample-win want got))))

(malloced-steady  ;; on a destroyed window
 (lambda ()
   (let* ((win    (gdk-window-new #f 100 100 '() 'input-output 'toplevel))
	  (want   '(#f NONE 0))
	  (got    (begin
		    (gdk-window-destroy win)
		    (gdk-selection-property-get win))))
     ;; final free()s only take place under the main loop, so need this to
     ;; satisfy `malloced-steady
     (gtk-main-iteration-do #f)
     (test "gdk-selection-property-get" (list "destroyed win" win) want got))))

;;
;; gdk-text-extents
;;

(malloced-steady
 (lambda ()
   (let* ((args   (list sample-font ""))
	  (want   '(0 0 0 0 0))
	  (got    (apply gdk-text-extents args)))
     (test "gdk-text-extents" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args   (list sample-font "ABC"))
	  (want   #t)
	  (got    (list? (apply gdk-text-extents args))))
     (test "gdk-text-extents" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args   (list sample-font (string #\nul)))
	  (want   #t)
	  (got    (list? (apply gdk-text-extents args))))
     (test "gdk-text-extents" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args   (list sample-font (make-shared-substring "ABC" 1 1)))
	  (want   '(0 0 0 0 0))
	  (got    (apply gdk-text-extents args)))
     (test "gdk-text-extents" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args   (list sample-font 99))
	  (want   #f)
	  (got    (false-if-exception (begin
					(apply gdk-text-extents args)
					#t))))
     (test "gdk-text-extents" args want got))))

;;
;; gdk-text-height
;; gdk-text-measure
;; gdk-text-width
;;

(let ()
  (define (text-tests name func)
    (malloced-steady
     (lambda ()
       (let* ((args   (list sample-font ""))
	      (want   0)
	      (got    (apply func args)))
	 (test name args want got))))

    (malloced-steady
     (lambda ()
       (let* ((args   (list sample-font "ABC"))
	      (want   #t)
	      (got    (number? (apply func args))))
	 (test name args want got))))

    (malloced-steady
     (lambda ()
       (let* ((args   (list sample-font (string #\nul)))
	      (want   #t)
	      (got    (number? (apply func args))))
	 (test name args want got))))

    (malloced-steady
     (lambda ()
       (let* ((args   (list sample-font (make-shared-substring "ABC" 1 1)))
	      (want   0)
	      (got    (apply func args)))
	 (test name args want got))))

    (malloced-steady
     (lambda ()
       (let* ((args   (list sample-font 99))
	      (want   #f)
	      (got    (false-if-exception (begin
					    (apply func args)
					    #t))))
	 (test name args want got)))))

  (text-tests "gdk-text-height"  gdk-text-height)
  (text-tests "gdk-text-measure" gdk-text-measure)
  (text-tests "gdk-text-width"   gdk-text-width))

;;
;; gdk-text-property-to-text-list
;;

(malloced-steady
 (lambda ()
   (let* ((args   (list 'STRING 8 (string #\A #\nul)))
	  (want   '("A"))
	  (got    (apply gdk-text-property-to-text-list args)))
     (test "gdk-text-property-to-text-list" args want got))))

;;
;; gdk-window?
;;

(malloced-steady
 (lambda ()
   (let* ((obj  sample-win)
	  (want #t)
	  (got  (gdk-window? obj)))
     (test "gdk-window?" obj want got))))

(malloced-steady
 (lambda ()
   (let* ((obj  123)
	  (want #f)
	  (got  (gdk-window? obj)))
     (test "gdk-window?" obj want got))))

(malloced-steady
 (lambda ()
   (let* ((obj  sample-font)
	  (want #f)
	  (got  (gdk-window? obj)))
     (test "gdk-window?" obj want got))))

;;
;; gdk-window-get-children
;;

(malloced-steady
 (lambda ()
   (let* ((win    sample-win)
	  (want   '())
	  (got    (gdk-window-get-children win)))
     (test "gdk-window-get-children sample-win" win want got))))

(malloced-steady
 (lambda ()
   (let* ((parent  (gdk-window-new #f 100 100 '() 'input-output 'toplevel))
	  (child-1 (gdk-window-new parent 50 60 '() 'input-output 'child))
	  (child-2 (gdk-window-new parent 70 80 '() 'input-output 'child))
	  (want    '((70 . 80) (50 . 60)))
	  ;; can't compare with eq?, so look at sizes
	  (got     (map gdk-window-get-size
			(gdk-window-get-children parent))))
     (gdk-window-destroy parent)
     (gdk-window-destroy child-1)
     (gdk-window-destroy child-2)
     ;; final free()s only take place under the main loop, so need this to
     ;; satisfy `malloced-steady
     (gtk-main-iteration-do #f)
     (test "gdk-window-get-children 2-children" parent want got))))


;;
;; gdk-window-get-events
;; gdk-window-set-events
;;

;; The result is only tested with a memq because firstly `structure-mask'
;; gets added in any gdk-window-set-events, and secondly there's some
;; nonsense in gdk 1.2 with `button-press-mask' and `button-release-mask' --
;; if you ask for either one then the return value from
;; gdk-window-get-events says you selected both (which you didn't).
(for-each (lambda (mask)
	    (malloced-steady
	     (lambda ()
	       (gdk-window-set-events sample-win (list mask))
	       (let* ((want #t)
		      (got  (->bool
			     (memq mask (gdk-window-get-events sample-win)))))
		 (test "gdk-window-get-events" mask want got)))))

	  '(exposure-mask
	    pointer-motion-mask
	    pointer-motion-hint-mask
	    button-motion-mask
	    button1-motion-mask
	    button2-motion-mask
	    button3-motion-mask
	    button-press-mask
	    button-release-mask
	    key-press-mask
	    key-release-mask
	    enter-notify-mask
	    leave-notify-mask
	    focus-change-mask
	    structure-mask
	    property-change-mask
	    visibility-notify-mask
	    ;; proximity-in-mask   ;; does nothing in gdk 1.2
	    ;; proximity-out-mask  ;; does nothing in gdk 1.2
	    substructure-mask))

;;
;; gdk-window-get-id
;;

(malloced-steady
 (lambda ()
   (let* ((id   (gdk-window-get-id sample-win))
	  (want #t)
	  (got  (exact-positive-integer? id)))
     (test "gdk-window-get-id" sample-win want got))))

;;
;; gdk-window-new
;;

(malloced-steady
 (lambda ()
   (let* ((args '(#f 100 100 () input-output toplevel))
	  (want #t)
	  (win  (apply gdk-window-new args))
	  (got  (->bool win)))
     (gdk-window-destroy win)
     ;; final free()s only take place under the main loop, so need this to
     ;; satisfy `malloced-steady'
     (gtk-main-iteration-do #f)
     (test "gdk-window-new" args want got))))

(malloced-steady ;; with title
 (lambda ()
   (let* ((args '(#f 100 100 () input-output toplevel #:title "foo"))
	  (want #t)
	  (win  (apply gdk-window-new args))
	  (got  (->bool win)))
     (gdk-window-destroy win)
     ;; final free()s only take place under the main loop, so need this to
     ;; satisfy `malloced-steady'
     (gtk-main-iteration-do #f)
     (test "gdk-window-new" args want got))))

(malloced-steady ;; with wmclass
 (lambda ()
   (let* ((args '(#f 100 100 () input-output toplevel
		     #:wmclass "my-name" "my-class"))
	  (want (string-append "my-name" (string #\nul)
			       "my-class" (string #\nul)))
	  (win  (apply gdk-window-new args))
	  (got  (first (gdk-property-get win 'WM_CLASS 'STRING 0 100 #f))))
     (gdk-window-destroy win)
     ;; final free()s only take place under the main loop, so need this to
     ;; satisfy `malloced-steady
     (gtk-main-iteration-do #f)
     (test "gdk-window-new" args want got))))

(malloced-steady ;; bad args
 (lambda ()
   (let* ((args '(#f 100 100 () input-output toplevel #:foo))
	  (want #f)
	  (win  (false-if-exception (apply gdk-window-new args)))
	  (got  (first (gdk-property-get win 'WM_CLASS 'STRING 0 100 #f))))
     (test "gdk-window-new" args want got))))

;;
;; gdk-window-set-geometry-hints
;;

(malloced-steady
 (lambda ()
   (let* ((args (list sample-win #:base-size 100 100))
	  (want #t)
	  (got  (begin
		  (apply gdk-window-set-geometry-hints args)
		  #t)))
     (test "gdk-window-set-geometry-hints" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args (list sample-win #:foo))
	  (want #f)
	  (got  (false-if-exception 
		 (begin
		   (apply gdk-window-set-geometry-hints args)
		   #t))))
     (test "gdk-window-set-geometry-hints" args want got))))

;;
;; gdk-window-set-hints
;;

(malloced-steady  ;; flags '()
 (lambda ()
   (let* ((args (list sample-win 0 0 100 100 100 100 '()))
	  (want #t)
	  (got  (begin
		  (apply gdk-window-set-hints args)
		  #t)))
     (test "gdk-window-set-hints" args want got))))

(malloced-steady  ;; flags 0
 (lambda ()
   (let* ((args (list sample-win 0 0 100 100 100 100 0))
	  (want #t)
	  (got  (begin
		  (apply gdk-window-set-hints args)
		  #t)))
     (test "gdk-window-set-hints" args want got))))

(malloced-steady  ;; flags list 0
 (lambda ()
   (let* ((args (list sample-win 0 0 100 100 100 100 '(0)))
	  (want #t)
	  (got  (begin
		  (apply gdk-window-set-hints args)
		  #t)))
     (test "gdk-window-set-hints" args want got))))

(malloced-steady  ;; flags list sym
 (lambda ()
   (let* ((args (list sample-win 0 0 100 100 100 100 '(pos)))
	  (want #t)
	  (got  (begin
		  (apply gdk-window-set-hints args)
		  #t)))
     (test "gdk-window-set-hints" args want got))))

(malloced-steady  ;; flags list sym 0
 (lambda ()
   (let* ((args (list sample-win 0 0 100 100 100 100 '(pos 0)))
	  (want #t)
	  (got  (begin
		  (apply gdk-window-set-hints args)
		  #t)))
     (test "gdk-window-set-hints" args want got))))

(malloced-steady  ;; flags frac
 (lambda ()
   (let* ((args (list sample-win 0 0 100 100 100 100 0.5))
	  (want #f)
	  (got  (false-if-exception (apply gdk-window-set-hints args))))
     (test "gdk-window-set-hints" args want got))))

(malloced-steady  ;; flags string
 (lambda ()
   (let* ((args (list sample-win 0 0 100 100 100 100 "pos"))
	  (want #f)
	  (got  (false-if-exception (apply gdk-window-set-hints args))))
     (test "gdk-window-set-hints" args want got))))

(malloced-steady  ;; flags list frac
 (lambda ()
   (let* ((args (list sample-win 0 0 100 100 100 100 '(0.5)))
	  (want #f)
	  (got  (false-if-exception (apply gdk-window-set-hints args))))
     (test "gdk-window-set-hints" args want got))))

(malloced-steady  ;; flags list string
 (lambda ()
   (let* ((args (list sample-win 0 0 100 100 100 100 '("pos")))
	  (want #f)
	  (got  (false-if-exception (apply gdk-window-set-hints args))))
     (test "gdk-window-set-hints" args want got))))

(malloced-steady  ;; flags dotted list
 (lambda ()
   (let* ((args (list sample-win 0 0 100 100 100 100 '(0 . 0)))
	  (want #f)
	  (got  (false-if-exception (apply gdk-window-set-hints args))))
     (test "gdk-window-set-hints" args want got))))

(malloced-steady  ;; flags integer too big
 (lambda ()
   (let* ((args (list sample-win 0 0 100 100 100 100 9999999999999999999))
	  (want #f)
	  (got  (false-if-exception (apply gdk-window-set-hints args))))
     (test "gdk-window-set-hints" args want got))))

(malloced-steady  ;; flags list integer too big
 (lambda ()
   (let* ((args (list sample-win 0 0 100 100 100 100 '(9999999999999999999)))
	  (want #f)
	  (got  (false-if-exception (apply gdk-window-set-hints args))))
     (test "gdk-window-set-hints" args want got))))


(tests-end)
