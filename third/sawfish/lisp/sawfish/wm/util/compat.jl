;; compat.jl -- aliases for obsolete functions
;; $Id: compat.jl,v 1.1.1.2 2003-01-05 00:32:24 ghudson Exp $

;; Copyright (C) 1999 John Harper <john@dcs.warwick.ac.uk>

;; This file is part of sawmill.

;; sawmill is free software; you can redistribute it and/or modify it
;; under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.

;; sawmill is distributed in the hope that it will be useful, but
;; WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with sawmill; see the file COPYING.  If not, write to
;; the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

(define-structure sawfish.wm.util.compat

    (export show-message
	    ws-copy-window
	    ws-move-window
	    ws-insert-workspace
	    ws-remove-workspace
	    sawmill-directory
	    sawmill-lisp-lib-directory
	    sawmill-site-lisp-directory
	    sawmill-exec-directory
	    sawmill-version
	    custom-set-color
	    custom-set-font
	    custom-set-frame-style)

    (open rep
	  sawfish.wm.misc
	  sawfish.wm.custom
	  sawfish.wm.commands
	  sawfish.wm.workspace)

;;; obsolete functions

  (define (show-message #!optional text font fg bg position)
    (let ((attrs nil))
      (when font
	(setq attrs (cons (cons 'font font) attrs)))
      (when fg
	(setq attrs (cons (cons 'fg fg) attrs)))
      (when bg
	(setq attrs (cons (cons 'bg bg) attrs)))
      (when position
	(setq attrs (cons (cons 'position position) attrs)))
      (display-message text attrs)))

  (define ws-copy-window copy-window-to-workspace)
  (define ws-move-window move-window-to-workspace)
  (define ws-insert-workspace insert-workspace)
  (define ws-remove-workspace remove-workspace)

;;; obsolete variables

  (define sawmill-directory sawfish-directory)
  (define sawmill-lisp-lib-directory sawfish-lisp-lib-directory)
  (define sawmill-site-lisp-directory sawfish-site-lisp-directory)
  (define sawmill-exec-directory sawfish-exec-directory)
  (define sawmill-version sawfish-version)

;;; obsolete commands

  (define (define-commands index)
    (let ((fn (lambda (base)
		(intern (format nil "%s:%d" base (1+ index))))))
      (define-command (fn "select-workspace")
	(lambda () (select-workspace-from-first index))
	#:class 'deprecated)
      (define-command (fn "send-to-workspace")
	(lambda (w) (send-window-to-workspace-from-first w index))
	#:spec "%W"
	#:class 'deprecated)
      (define-command (fn "copy-to-workspace")
	(lambda (w) (send-window-to-workspace-from-first w index t))
	#:spec "%W"
	#:class 'deprecated)))

  (do ((i 0 (1+ i)))
      ((= i 9))
    (define-commands i))

  (define-command 'insert-workspace (command-ref 'insert-workspace-after)
    #:class 'deprecated)

;;; obsolete options

  (mapc (lambda (x)
	  (put x 'custom-obsolete t))
	'(viewport-columns viewport-rows viewport-dimensions
	  preallocated-workspaces iconify-whole-group
	  uniconify-whole-group always-update-frames
	  cycle-disable-auto-raise cycle-show-window-icons
	  cycle-warp-pointer cycle-focus-windows cycle-raise-windows
	  edge-flip-warp-pointer frame-type-fallbacks
	  warp-to-window-x-offset warp-to-window-y-offset
	  uniquify-name-format transients-get-focus decorate-transients
	  raise-windows-on-uniconify uniconify-to-current-workspace
	  uniconify-to-current-viewport iconify-ignored
	  maximize-always-expands maximize-ignore-when-filling
	  maximize-avoid-avoided focus-windows-on-uniconify
	  transients-are-group-members raise-selected-windows
	  warp-to-selected-windows menus-include-shortcuts
	  configure-auto-gravity configure-ignore-stacking-requests
	  beos-window-menu-simplifies customize-show-symbols
	  tooltips-timeout-enabled tooltips-delay
	  tooltips-timeout-delay tooltips-font
	  tooltips-foreground-color tooltips-background-color
	  move-snap-mode move-snap-ignored-windows
	  move-resize-inhibit-configure move-snap-edges
	  raise-windows-when-unshaded persistent-group-ids
	  delete-workspaces-when-empty transients-on-parents-workspace
	  edge-flip-delay audio-for-ignored-windows
	  size-window-def-increment slide-window-increment
	  default-bevel-percent sp-padding nokogiri-user-level
	  nokogiri-buttons workspace-boundary-mode
	  workspace-send-boundary-mode lock-first-workspace
	  ignore-window-input-hint default-window-animator
	  resize-edge-mode move-outline-mode resize-outline-mode
	  move-resize-raise-window workspace-geometry
	  pointer-motion-threshold ignore-program-positions))

;;; obsolete custom setters

  (define (custom-set-color var value #!optional req)
    (custom-set-typed-variable var value 'color req))
  (define (custom-set-font var value #!optional req)
    (custom-set-typed-variable var value 'font req))
  (define (custom-set-frame-style var value #!optional req)
    (custom-set-typed-variable var value 'frame-style req))

  (define-custom-setter 'custom-set-color custom-set-color)
  (define-custom-setter 'custom-set-font custom-set-font)
  (define-custom-setter 'custom-set-frame-style custom-set-frame-style))