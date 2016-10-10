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

(use-modules (gtk gtk))

(define titles #("name" "uid" "gid" "passwd" "gecos" "home" "shell"))

(define window (gtk-window-new 'toplevel))
(define scrolled-window (gtk-scrolled-window-new))
(define clist (gtk-clist-new-with-titles titles))
(gtk-container-add window scrolled-window)
(gtk-container-add scrolled-window clist)

(let loop ((pw (getpwent)))
  (cond (pw
	 (gtk-clist-append clist
			   (vector (passwd:name pw)
				   (number->string (passwd:uid pw))
				   (number->string (passwd:gid pw))
				   (passwd:passwd pw)
				   (passwd:gecos pw)
				   (passwd:dir pw)
				   (passwd:shell pw)))
	 (loop (getpwent)))))

(do ((i 0 (1+ i)))
    ((>= i (vector-length titles)))
  (gtk-clist-set-column-auto-resize clist i #t))

(gtk-widget-show-all window)

(gtk-standalone-main window)
