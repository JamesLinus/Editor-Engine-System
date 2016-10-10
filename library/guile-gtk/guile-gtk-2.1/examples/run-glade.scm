#!/bin/sh
exec guile -s "$0" "$@"
!#

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
  (ice-9 getopt-long)
  (gtk-2.0 gtk)
  (gtk-2.0 glade))

(define-macro (use-modules/ext . modules)
  `(eval '(use-modules ,@modules) (current-module)))

(define-macro (when cond . body)
  `(if ,cond (begin ,@body)))

(define args (getopt-long (program-arguments) '((my-connect) (init-gnome) (init-bonobo) )))

(when (assq-ref args 'init-gnome)
  (use-modules/ext (gnome gnome))
  ; I really hate functions which tries do do million things at once
  ; init gnome, parse arguments, dance, sing,... 
  (gnome-init-hack "simple-glade" (lambda args #t)
		   '(("init-gnome" "Use glade-gnome" "")
		     ("init-bonobo" "Use glade-bonobo" "")
		     ("my-connect" "Use my-connect instead default one...")))
  (glade-gnome-init))

(when (assq-ref args 'init-bonobo)
  (use-modules/ext (gnome bonobo))
  (glade-bonobo-init))

(and (null? (assq-ref args '()))
     (error "No glade files"))

(define xmls (map glade-xml-new (assq-ref args '())))

(define my-connect (and (assq-ref args 'my-connect)
	(lambda (handler object signal-name signal-data connect-object after)
	  (format #t "Connecting (~s,~s) => ~s~%" object signal-name handler)
	  (gtk-signal-connect (or connect-object object)
			      signal-name (eval-string handler)))))

(define (find-widget name)
  (or-map (lambda (xml) (glade-xml-get-widget xml name)) xmls))

(for-each (lambda (xml) (glade-xml-signal-autoconnect-full xml my-connect)) xmls)

(gtk-main)
