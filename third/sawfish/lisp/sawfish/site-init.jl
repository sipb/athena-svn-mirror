;; $Id: site-init.jl,v 1.1 2000-12-27 22:35:45 ghudson Exp $
;; Site initialization for Athena

;; FOCUS MODE
(require 'auto-raise)
(setq focus-click-through nil)
(setq focus-mode 'click)
(setq raise-windows-on-focus t)
(setq raise-window-timeout 0)

;; KEY BINDINGS
(unbind-keys border-keymap "Button3-Off")
(bind-keys border-keymap "Button3-Click" 'popup-window-menu)
(bind-keys window-keymap "Button1-Click1" 'raise-window-and-pass-through-click
			 "M-Button2-Click1" 'raise-lower-window
			 "M-Button3-Click1" 'popup-window-menu)
(unbind-keys title-keymap "Button1-Click2" "Button2-Move" "Button3-Off")
(bind-keys title-keymap "Button3-Click1" 'popup-window-menu)
(unbind-keys global-keymap "C-Left" "C-Right")

;; FRAME STYLE
(add-window-matcher 'WM_NAME "zwgc" '(never-focus . t)
				    '(skip-tasklist . t)
				    '(sticky-viewport . t)
				    '(frame-type . border-only)
				    '(depth . 1))

;; MENUS
(require 'menus)
(require 'old-window-menu)
(setq menu-program-stays-running t)
(menu-start-process)

(setq window-ops-menu
      '(("Raise" raise-window)
	("Lower" lower-window)
	("Iconify" iconify-window)
	("Maximize" maximize-window)
	("Unmaximize" unmaximize-window)
	("Close" delete-window)
	("Close Forcibly" destroy-window)
	()
	("Frame type"
	 ("Normal" set-frame:default)
	 ("Title-only" set-frame:shaped)
	 ("Border-only" set-frame:transient)
	 ("Top-border" set-frame:shaped-transient)
	 ("None" set-frame:unframed))
	("Frame style" . frame-style-menu)))

;; WINDOW HISTORY
(require 'window-history)
(setq window-history-auto-save-position nil)
(setq window-history-auto-save-dimensions nil)
(setq window-history-auto-save-state nil)
