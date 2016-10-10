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

(define-module (gtk-2.0 gtk)
  :use-module (gtk-2.0 dynlink)
  :use-module (gtk-2.0 gdk))

(merge-compiled-code "sgtk_init_gtk_gtk_glue" "libguilegtk-2.0")

(define-public (gtk-update)
  (cond ((> (gtk-events-pending) 0)
	 (gtk-main-iteration)
	 (gtk-update))))

(define-public (gtk-standalone-main toplevel)
  (cond ((gtk-standalone?)
	 (gtk-signal-connect toplevel "destroy" gtk-exit)
	 (gtk-main))))

(define-public (gtk-pixmap-new-from-file file parent)
  (let ((mask (vector #f)))
    (gtk-pixmap-new (gdk-pixmap-colormap-create-from-xpm 
		      #f (gtk-widget-get-colormap parent) mask #f file)
		    (vector-ref mask 0))))

;; Some aliases and quickies

(define-public gtk-radio-menu-item-new 
  gtk-radio-menu-item-new-from-widget)
(define-public gtk-radio-menu-item-new-with-label
  gtk-radio-menu-item-new-with-label-from-widget)
(define-public gtk-radio-button-new
  gtk-radio-button-new-from-widget)
(define-public gtk-radio-button-new-with-label
  gtk-radio-button-new-with-label-from-widget)
(define-public (gtk-idle-add proc)
  (gtk-idle-add-full gtk-priority-default proc))
(define-public gtk-idle-add-priority gtk-idle-add-full)

;; GtkDialog

(define-public (gtk-dialog-new-with-buttons title parent flags . text+ids)
  (let ((dialog (%gtk-dialog-new-with-buttons title parent flags)))
    (apply gtk-dialog-add-buttons dialog text+ids)
    dialog))

(define-public (gtk-dialog-add-buttons dialog . text+ids)
  (or (null? text+ids)
      (let ((text (car text+ids))
	    (id   (cadr text+ids)))
	(gtk-dialog-add-button dialog text id)
	(apply gtk-dialog-add-buttons dialog (cddr text+ids)))))

;; The error reporter

;; For reference, some versions of gtk circa 2.8 or 2.10 spat out g_log
;; warning messages about font stuff relating to the GtkText here.  Believe
;; that was some sort of breakage in gtk itself, some incompatibility in the
;; ongoing half-support for GtkText (it's supposed to be superceded by
;; GtkTextView etc).  The messages seemed harmless, and have gone away in
;; gtk 2.12.
;;
(define-public gtk-show-error
  (let ((window #f)
	(text #f))
    (lambda (msg)
      (cond ((not window)
	     (set! window (gtk-window-new 'toplevel))
	     (set! text (gtk-text-new #f #f))
	     (let* ((vscroll (gtk-vscrollbar-new (gtk-text-vadj text)))
		    (close (gtk-button-new-with-label "Close"))
		    (hbox (gtk-hbox-new #f 1))
		    (vbox (gtk-vbox-new #f 3)))

	       (gtk-container-add window vbox)
	       (gtk-box-pack-start vbox hbox #t #t 0)
	       (gtk-box-pack-start hbox text #t #t 0)
	       (gtk-box-pack-start hbox vscroll #f #t 0)
	       (gtk-box-pack-start vbox close #f #t 0)
	       (gtk-window-set-title window "guile-gtk error messages")
	       (gtk-widget-set-usize window 320 200)
	       (gtk-window-set-policy window #t #t #f)
	       (gtk-signal-connect close "clicked"
				   (lambda () (gtk-widget-destroy window)))
	       (gtk-signal-connect window "destroy"
				   (lambda () 
				     (set! window #f)
				     (set! text #f)))
	       (gtk-widget-show-all window))))
      (gtk-text-insert text #f #f #f msg -1))))

(define (call-with-error-catching thunk)
  (let ((the-last-stack #f)
	(stack-saved? #f))
    
    (define (handle-error key args)
      (let ((text (call-with-output-string
		   (lambda (cep)
		     (if the-last-stack
			 (display-backtrace the-last-stack cep)
			 (display "no backtrace available.\n" cep))
		     (apply display-error the-last-stack cep args)))))
	(gtk-show-error text)
	#f))

    (define (save-stack)
      (cond (stack-saved?)
	    ((not (memq 'debug (debug-options-interface)))
	     (set! the-last-stack #f)
	     (set! stack-saved? #t))
	    (else
	     (set! the-last-stack (make-stack #t lazy-dispatch 4))
	     (set! stack-saved? #t))))

    (define (lazy-dispatch key . args)
      (save-stack)
      (apply throw key args))

    (start-stack #t
		 (catch #t
			(lambda ()
			  (lazy-catch #t
				      thunk
				      lazy-dispatch))
			(lambda (key . args)
			  (if (= (length args) 4)
			      (handle-error key args)
			      (apply throw key args)))))))

(define-macro (with-error-catching . body)
  `(call-with-error-catching (lambda () ,@body)))

(gtk-callback-trampoline (lambda (proc args)
			   (with-error-catching
			    (apply proc args))))
