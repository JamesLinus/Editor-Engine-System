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

(define-module (gtk-2.0 dynlink)
  :use-module (gtk-2.0 config))

;; Merge some compiled code into the public interface of the current
;; module.  The compiled code will be initialized by calling a
;; function named INIT-FUNC in the library called LIBNAME.

(define-public (merge-compiled-code init-func libname)
  (let* ((module (current-module))
	 (interface (module-public-interface module)))
    ;; make the new primitives visible from within the current module.
    (module-use! module interface) ; XXX - is this safe?
    (save-module-excursion
     (lambda ()
       (set-current-module interface)
       (load-extension libname init-func)))))

(define-public (module-alias mod-name)
  (let ((mod-iface (resolve-interface mod-name)))
    (or mod-iface
	(error "no such module" mod-name))
    (set-module-public-interface! (current-module) mod-iface)))
