; -*- Scheme -*-

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

(register-type
 'GdkGLConfigList
 (make-type "GdkGLConfigList" "int*"
            #f
            (lambda (x pos subr)
	      (@@ "sgtk_scm2gtk_gl_config(~a,~a,~a)" x pos subr))
	    #f
	    'scm2c-does-type-checking #t
	    'finish (lambda (x y)
		      (@@ "sgtk_gl_config_finish(~a)" x))))
