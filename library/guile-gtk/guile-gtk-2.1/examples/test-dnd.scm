;; Copyright (C) 2002, 2006 Free Software Foundation, Inc.
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

(use-modules
  (gtk-2.0 gdk)
  (gtk-2.0 gtk))

; CLEVER scheme coder would write some schemish envelope around C
; oriented gtk-drag* interface. However this is straight transcription
; of testdnd.c, so we left clever schemish interface as an excercise
; to reader :-)

(define TARGET-STRING 0)
(define TARGET-ROOTWIN 1)
(define TARGET-URL 2)

(define *targets-list*
  `(("STRING" 0 ,TARGET-STRING)
    ("text/plain" 0 ,TARGET-STRING)
    ("text/uri-list" 0 ,TARGET-URL)))

(define *targets-root-list* (append *targets-list*
  `(("application/x-rootwin-drop" 0 ,TARGET-ROOTWIN))))

;--- Utils ... --------------------------------------------------------

(define-macro (unless cond . body)
  `(or ,cond (begin ,@body)))

(define-macro (when cond . body)
  `(and ,cond (begin ,@body)))

(define (report . args)
  (display "DND: ")
  (apply format #t args)
  (newline))

(define (load-xpm file)
  (let ((bitmap (vector #f)))
    (cons 
     (gdk-pixmap-colormap-create-from-xpm #f (gtk-widget-get-colormap *window*) bitmap #f file)
     (vector-ref bitmap 0))))

(define xpm-pixmap car)
(define xpm-bitmap cdr)

(define (pixmap-new xpm)
  (gtk-pixmap-new (xpm-pixmap xpm) (xpm-bitmap xpm)))

(define (pixmap-set pixmap xpm)
  (gtk-pixmap-set pixmap (xpm-pixmap xpm) (xpm-bitmap xpm)))

;--- the rest ... :-) -------------------------------------------------

(define *window* (gtk-window-new 'toplevel))
(gtk-signal-connect *window* "destroy" gtk-main-quit)

(define *drag-icon* (load-xpm "drag-icon.xpm"))
(define *trashcan-open* (load-xpm "trashcan-open.xpm"))
(define *trashcan-closed* (load-xpm "trashcan-closed.xpm"))

	; just for debugging...
(define *invalidated* #f)

(let ((table (gtk-table-new 2 2 #f)))

  (gtk-container-add *window* table)

  (let ((label (gtk-label-new "Drop Here\n")))
    (gtk-drag-dest-set label '(all) *targets-list* '(copy move))
    (gtk-signal-connect label "drag_data_received"
	(lambda (context x y data info time)
	  (report "Received ~s in label" (gtk-selection-data-data data))))

    (gtk-table-attach table label 0 1 0 1))

  (let ((button (gtk-button-new-with-label "Drag Here\n")))
    (gtk-drag-source-set button '(button1-mask button3-mask) *targets-root-list* '(copy move))
    (gtk-drag-source-set-icon button (gtk-widget-get-colormap *window*)
			      (xpm-pixmap *drag-icon*) (xpm-bitmap *drag-icon*))

    (gtk-signal-connect button "drag_data_get"
       (lambda (context data info time)
	 (set! *invalidated* data)
	 (cond 
	   ((eq? info TARGET-ROOTWIN)
	    (report "I was dropped on the rootwin"))
	   ((eq? info TARGET-URL)
	    (gtk-selection-data-set data (gtk-selection-data-target data)
				    8 "http://www.gnu.org/software/guile"))
	   (else
	    (gtk-selection-data-set data (gtk-selection-data-target data)
				    8 "I'am Data from Guile program!")))))

    (gtk-signal-connect button "drag_data_delete"
      (lambda (context)
	(report "Delete the data!")))
    
    (gtk-table-attach table button 0 1 1 2))
  
  (let ((trash (pixmap-new *trashcan-closed*))
	(drag? #f))
    
    (define (close)
      (set! drag? #f)
      (pixmap-set trash *trashcan-closed*))

    (define (open)
      (unless drag?
	(set! drag? #t)
	(pixmap-set trash *trashcan-open*)))
    
    (gtk-drag-dest-set trash 0 '() 0)

    (gtk-signal-connect trash "drag_leave"
       (lambda (context time)
	 (report "Leave")
	 (close)))

    (gtk-signal-connect trash "drag_motion"
       (lambda (context x y time)
	 (open)
	 (report "motion, source ~a" (gtk-drag-get-source-widget context))
	 (gdk-drag-status context (gdk-drag-context-suggested-action context) time)
	 #t))

    (gtk-signal-connect trash "drag_drop"
       (lambda (context x y time)
	 (let ((targets (gdk-drag-context-targets context)))
	   (report "drop, targets ~s" targets)
	   (close)
	   (when (pair? targets)
	     (report "getting")
	     (gtk-drag-get-data trash context (car targets) time)
	     #t))))
    
    (gtk-signal-connect trash "drag_data_received"
       (lambda (context x y data info time)
	 (report "Received ~a in trashcan" (gtk-selection-data-data data))
	 (gtk-drag-finish context #t #f time)))
	   
    (gtk-table-attach table trash 1 2 0 1)))
  
(gtk-widget-show-all *window*)
(gtk-main)
