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
      (display "Cannot run gdk-pixbuf tests without an X display\n")
      (exit 77)))

;; the source tree doesn't have gdk-pixbuf.scm under the gtk-2.0 directory,
;; so can't use-modules it, instead load it directly
(primitive-load (string-append (getenv "srcdir")
			       "/../gdk-pixbuf/gdk-pixbuf.scm"))
;; then switch back to the normal guile-user module
(define-module (guile-user))

(use-modules (gtk-2.0 gdk-pixbuf))


;;
;; gdk-pixbuf-new
;;

(malloced-steady
 (lambda ()
   (let* ((args '(rgb #f 8 1 1))
	  (want #t)
	  (got  (->bool (apply gdk-pixbuf-new args))))
     (test "gdk-pixbuf-new" args want got))))


;;
;; gdk-pixbuf-get-colorspace
;;

(malloced-steady
 (lambda ()
   (let* ((args   '(rgb #f 8 1 1))
	  (pixbuf (apply gdk-pixbuf-new args))
	  (want   'rgb)
	  (got    (gdk-pixbuf-get-colorspace pixbuf)))
     (test "gdk-pixbuf-get-colorspace" args want got))))

;;
;; gdk-pixbuf-get-rowstride
;;

(malloced-steady
 (lambda ()
   (let* ((args   '(rgb #f 8 1 1))
	  (pixbuf (apply gdk-pixbuf-new args))
	  (want   4)
	  (got    (gdk-pixbuf-get-rowstride pixbuf)))
     (test "gdk-pixbuf-get-rowstride" args want got))))

(malloced-steady
 (lambda ()
   (let* ((args   '(rgb #t 8 1 1))
	  (pixbuf (apply gdk-pixbuf-new args))
	  (want   4)
	  (got    (gdk-pixbuf-get-rowstride pixbuf)))
     (test "gdk-pixbuf-get-rowstride" args want got))))

;;
;; gdk-pixbuf-loader-new
;;

(malloced-steady
 (lambda ()
   (let* ((loader (gdk-pixbuf-loader-new))
	  (want   #t)
	  (got    (gdk-pixbuf-loader? loader)))
     (test "gdk-pixbuf-loader-new" #f want got))))


;;
;; gdk-pixbuf-loader-write
;;

(malloced-steady
 (lambda ()
   (let* ((loader (gdk-pixbuf-loader-new))
	  (want   #t)
	  (got    (false-if-exception
		   (begin
		     (gdk-pixbuf-loader-write loader
					      (string (integer->char 0)) 1)
		     #t))))
     (test "gdk-pixbuf-loader-write" #f want got))))



(tests-end)
