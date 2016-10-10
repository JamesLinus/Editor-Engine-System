;; Copyright (C) 2003, 2006 Free Software Foundation, Inc.
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

(define-module (gtk-2.0 threads)
  :use-module (gtk-2.0 gtk)
  :use-module (gtk-2.0 gdk)
  :use-module (ice-9 threads))

(export gtk-threads-handler? gtk-threads-ensure-handler)

(define handler-running? #f)

(define (gtk-threads-handler?)
  handler-running?)

(define (gtk-threads-ensure-handler)
  (if (not handler-running?)
      (begin-thread
       (dynamic-wind
	   (lambda ()
	     (gdk-threads-enter)
	     (set! handler-running? #t))
	   (lambda ()
	     (gtk-main))
	   (lambda ()
	     (set! handler-running? #f)
	     (gdk-threads-leave))))))
