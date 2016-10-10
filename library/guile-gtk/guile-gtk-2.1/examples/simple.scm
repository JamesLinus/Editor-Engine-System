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

(read-set! keywords 'prefix)

(let* ((window (gtk-widget-new 'GtkWindow
			       :type         'toplevel
			       :title        "hello world"
			       :allow_grow   #f
			       :allow_shrink #f
			       :GtkContainer::border_width 10))
       (label  (gtk-widget-new 'GtkLabel
			       :label        "hello world"
			       :visible      #t))
       (button (gtk-widget-new 'GtkButton
			       :child        label
			       :parent       window
			       :visible      #t)))

  (gtk-signal-connect button "clicked" 
		      (lambda ()
			(display (gtk-object-get label :label))
			(newline)
			(gtk-widget-set label :label "yo!")))
  (gtk-widget-show window)
  (gtk-standalone-main window))
