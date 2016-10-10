;; Copyright (C) 2002 Free Software Foundation, Inc.
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

(read-set! keywords 'prefix)

(use-modules (gtk gtk))

(define tictactoe-new
  (let ((ttt-class (gtk-class-new 'GtkVBox "TicTacToe")))
    (gtk-signal-new-generic "tictactoe" '(first) ttt-class 'void '())
    (lambda ()
      (let* ((widget (gtk-widget-new ttt-class))
	     (table (gtk-table-new 3 3 #t))
	     (buttons (make-vector 9)))
	(define (ttt-clear)
	  (do ((p 0 (1+ p)))
	      ((>= p 9))
	    (gtk-widget-set (vector-ref buttons p) :active #f)))
	(define (ttt-toggle)
	  (let loop ((wins '((0 1 2) (3 4 5) (6 7 8)
			     (0 3 6) (1 4 7) (2 5 8)
			     (0 4 8) (2 4 6))))
	    (cond ((not (null? wins))
		   (cond ((and-map (lambda (wp) 
				     (gtk-widget-get (vector-ref buttons wp)
						     :active))
				   (car wins))
			  (gtk-signal-emit widget "tictactoe")
			  (ttt-clear))
			 (else
			  (loop (cdr wins))))))))
	    
	(do ((p 0 (1+ p)))
	    ((>= p 9))
	  (let ((b (gtk-toggle-button-new))
		(i (quotient p 3))
		(j (remainder p 3)))
	    (vector-set! buttons p b)
	    (gtk-table-attach-defaults table b i (1+ i) j (1+ j))
	    (gtk-signal-connect b "toggled" ttt-toggle)
	    (gtk-widget-set-usize b 20 20)))
	(gtk-container-add widget table)
	(gtk-widget-show-all widget)
	widget))))

(define w (gtk-window-new 'toplevel))
(define ttt (tictactoe-new))
(gtk-container-add w ttt)
(gtk-widget-show-all w)
(gtk-signal-connect ttt "tictactoe" (lambda () (pk 'Yay)))

(gtk-standalone-main w)
