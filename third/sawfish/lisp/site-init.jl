;; $Id: site-init.jl,v 1.20 2003-05-29 16:33:17 ghudson Exp $
;; Site initialization for Athena

;; Pick a nice readable default font.
(setq default-font
      (get-font "-adobe-helvetica-bold-r-normal-*-*-140-*-*-p-*-iso8859-1"))

;; Set the default theme.
(setq default-frame-style 'Crux-athena)

;; Load gnome stuff even if we start before any gnome properties are set.
(require 'sawfish.wm.state.gnome)

;; Don't load a custom default file; it loses information about which
;; variables users have set, and it overrides site defaults.
(setq custom-default-file "/dev/null")

;; Define some of the settings from the sawfish custom default file as
;; site defaults.  What we left out was a setting to enable tooltips,
;; to disable showing of tooltip doc strings, and to set the worksapce
;; geometry to contain four workspaces.
(setq warp-to-selected-windows nil)
(setq cycle-warp-pointer nil)
(setq focus-mode 'click)
(bind-keys window-keymap "Button1-Click1" 'raise-and-pass-through-click)

;; Focus mode customizations
(require 'auto-raise)
(setq focus-windows-on-uniconify t)
(setq raise-window-timeout 0)

;; Key bindings
(unbind-keys border-keymap "Button3-Off")
(bind-keys border-keymap "Button3-Click" 'popup-window-menu)
(bind-keys window-keymap "W-Button2-Click1" 'raise-lower-window
			 "W-Button3-Click1" 'popup-window-menu)
(unbind-keys title-keymap "Button2-Move" "Button3-Off")
(bind-keys title-keymap "Button1-Click2" 'maximize-window-toggle
			"Button3-Click1" 'popup-window-menu)
(unbind-keys global-keymap "W-Left" "W-Right")
(bind-keys global-keymap "W-Esc" 'cycle-windows-backwards
			 "W-Space" 'popup-window-menu)

;; Special treatment for zwgc windows
(require 'sawfish.wm.ext.match-window)
(add-window-matcher 'WM_NAME "zwgc" '(never-focus . t)
				    '(window-list-skip . t)
				    '(task-list-skip . t)
				    '(sticky-viewport . t)
				    '(frame-type . border-only))
(add-window-matcher 'WM_NAME "Console" '(never-focus . t))

;; Workaround for Maple, which otherwise comes up with no border
;; for some reason.
(add-window-matcher 'WM_CLASS "maple" '(frame-type . normal))

;; Menu customizations
(require 'sawfish.wm.menus)
(require 'sawfish.wm.ext.old-window-menu)
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
	("Toggle" . window-ops-toggle-menu)
	("Frame type" . frame-type-menu)
	("Frame style" . frame-style-menu)))

(setq root-menu
      '(("_New terminal window" (system "gnome-terminal &"))
	("_Windows" . window-menu)
	("Work_spaces" . workspace-menu)
	("_Customize" . custom-menu)
	("_Help"
	 ("_Athena help" (system "exec help &"))
	 ("Sawfish _manual" help:show-programmer-manual)
	 ("About Sawfish..." help:about))
	()
	("Restart" restart)
	("Quit" quit)))

;; Window history
(require 'sawfish.wm.ext.window-history)
(setq window-history-auto-save-position nil)
(setq window-history-auto-save-dimensions nil)
(setq window-history-auto-save-state nil)
