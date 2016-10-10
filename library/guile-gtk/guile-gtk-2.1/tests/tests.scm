;; Shared setups for tests.

;; Copyright (C) 2003, 2004, 2005, 2006 Free Software Foundation, Inc.
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


(define option-trace #f)


;;-----------------------------------------------------------------------------

(use-modules (srfi srfi-1))

;; (if option-trace
;;     (begin
;;       (setvbuf (current-output-port) _IONBF)
;;       (setvbuf (current-error-port) _IONBF)))
  
;; our own extension
(load-extension (string-append (getcwd) "/tests") "tests_init")

(if (= 0 (mallinfo-uordblks))
    (display "Warning: no mallinfo(), memory leak checking will be minimal\n"))

;; top_srcdir for uninstalled gtk-2.0/gtk.scm etc
;; top_builddir for uninstalled gtk-2.0/config.scm
;;
(set! %load-path (cons* (string-append (getenv "srcdir") "/..")
			".."
			%load-path))

;; nasty hack to pick up the uninstalled libguilegtk-2.0.la when
;; gtk-2.0/gtk.scm, gdk-pixbuf/gdk-pixbuf.scm etc do `load-extension'
;;
(let ((path (getenv "LTDL_LIBRARY_PATH")))
  (if path
      (set! path (string-append ":" path))
      (set! path ""))
  (setenv "LTDL_LIBRARY_PATH" (string-append (getcwd) "/.." ":"
					     (getcwd) "/../gdk-pixbuf" ":"
					     (getcwd) "/../glade" ":"
					     (getcwd) "/../gtk-gl"
					     path)))
(let ((path (getenv "LD_LIBRARY_PATH")))
  (if path
      (set! path (string-append ":" path))
      (set! path ""))
  (setenv "LD_LIBRARY_PATH" (string-append (getcwd) "/.." ":"
					   (getcwd) "/../gdk-pixbuf" ":"
					   (getcwd) "/../glade" ":"
					   (getcwd) "/../gtk-gl"
					   path)))
			   

;;-----------------------------------------------------------------------------

(define tests-count 0)
(define test-last-name #f)

(define (test name data want got)
  (set! test-last-name name)
  (if option-trace
      (format #t "~a ~a:\n" name data))
  (set! tests-count (1+ tests-count))
  (if (not (equal? want got))
      (begin
	(format #t "~a:\n" name)
	(format #t "  data: ~s\n" data)
	(format #t "  want: ~s\n" want)
	(format #t "  got:  ~s\n" got)
	(exit 1))))

(define (tests-end)
  (format #t "  ~a tests ok\n" tests-count)
  (exit 0))


;;-----------------------------------------------------------------------------
;; malloc stuff

;; Call (THUNK) a few times, looking for `gc-stats' bytes-malloced to be
;; unchanged, after a few tries at least.
;;
;; This is designed to detect memory leaks or scm_done_malloc count leaks in
;; C code, a problem will show up as malloced increasing on every loop.
;;

(define (malloced-steady thunk)
  (define old-malloced -1)
  (define new-malloced -1)
  (define old-uordblks -1)
  (define new-uordblks -1)
  (define saw-decrease #f)

  (let more ((attempt 0))
    (if (> attempt 30)
	(if saw-decrease
	    (begin
	      (format #t "Fluctuating: ~a\n~s\n"
		      test-last-name
		      (procedure-source thunk))
	      (format #t "old-malloced ~a\n" old-malloced)
	      (format #t "new-malloced ~a\n" new-malloced)
	      (format #t "old-uordblks ~a\n" old-uordblks)
	      (format #t "new-uordblks ~a\n" new-uordblks)
	      (format #t "diff malloced ~a\n" (- new-malloced old-malloced))
	      (format #t "diff uordbllks ~a\n" (- new-uordblks old-uordblks))
	      #t)
	    (begin
	      (format #t "~a\n~s\n"
		      test-last-name
		      (procedure-source thunk))
	      (format #t "old-malloced ~a\n" old-malloced)
	      (format #t "new-malloced ~a\n" new-malloced)
	      (format #t "old-uordblks ~a\n" old-uordblks)
	      (format #t "new-uordblks ~a\n" new-uordblks)
	      (format #t "diff malloced ~a\n" (- new-malloced old-malloced))
	      (format #t "diff uordbllks ~a\n" (- new-uordblks old-uordblks))
	      (error "Malloc leak")
	      #f))
	(begin
	  (set! old-malloced new-malloced)
	  (set! old-uordblks new-uordblks)

	  (gc)
	  (gc)
	  (thunk)
	  (thunk)
	  (gc)
	  (gc)
	  (set! new-malloced (assoc-ref (gc-stats) 'bytes-malloced))
	  (set! new-uordblks (mallinfo-uordblks))

	  (if (and (= old-malloced new-malloced)
		   (= old-uordblks new-uordblks))
	      'steady
	      (begin
		(if (and (positive? old-uordblks)
			 (< old-uordblks new-uordblks))
		    (set! saw-decrease #t))
		(more (1+ attempt))))))))
