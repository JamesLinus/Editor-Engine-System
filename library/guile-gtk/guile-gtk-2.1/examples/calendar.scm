;; A small calendar application for Guile-gtk

;; Copyright (C) 1999 Free Software Foundation, Inc.
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


;;This is a simple calendar built using the GtkCalendar Widget
;;It actually does nothing but being a Calendar.

(use-modules (gtk gtk))

(define (calendar-example)
 (let ((window (gtk-window-new 'toplevel))
       (calendar (gtk-calendar-new)))
 (gtk-container-add window calendar)
 (gtk-widget-show-all window) 
 (gtk-standalone-main window) )) 

(calendar-example)

;;Ariel Rios
