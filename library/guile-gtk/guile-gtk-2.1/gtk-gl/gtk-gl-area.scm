; -*- Scheme -*-

;; Copyright (C) 2002, 2003, 2006 Free Software Foundation, Inc.
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

(define-module (gtk-2.0 gtk-gl-area)
  :use-module (gtk-2.0 dynlink)
  :use-module (gtk-2.0 gtk))

(merge-compiled-code "sgtk_init_gtk_gtk_gl_area_glue" "libguilegtkgl-2.0")

(define-public gtk-gl-area-swapbuffers gtk-gl-area-swap-buffers)

(define-public (gtk-gl-area-make-current-force area)
  (if (not (gtk-gl-area-make-current area))
      (error "gtk-gl-area-make-current failed")))
      
(defmacro-public with-render-to-gl-area (area . body)
  `(begin
     (gtk-gl-area-make-current-force ,area)
     ,@body
     (gtk-gl-area-swap-buffers ,area)))

(defmacro-public with-gl-area (area . body)
  `(begin
     (gtk-gl-area-make-current-force ,area)     
     ,@body))


