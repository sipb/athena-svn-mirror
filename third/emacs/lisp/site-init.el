; Some gnus settings.  We set gnus-mode-non-string-length to 27 to make room
; for line-number-mode; this seems to be an oversight in the defaults
; (understandable since line-number-mode wasn't on by default until 19.30).
; We set gnus-use-long-filename to t to force consistent filenames on all
; platforms, overriding the backward-compatible default for System V.
(setq news-inews-program "/afs/sipb.mit.edu/project/sipb/bin/inews"
      gnus-default-nntp-server "news.mit.edu"
      gnus-local-organization "Massachusetts Institute of Technology"
      gnus-mode-non-string-length 27
      gnus-use-long-file-name t)

; mh-rmail won't find mh programs by default; tell it where they are.
(setq mh-progs "/usr/athena/bin/"
      mh-lib "/usr/athena/etc/")

; Security measure.
(setq inhibit-local-variables t)

; Handle bug reports locally.
(setq bug-gnu-emacs "bugs@mit.edu")

; Force outgoing mail domain to be "mit.edu" instead of local machine name.
(setq mail-host-address "mit.edu")

; Force print command to be "lpr" on SysV sytems; "lp" doesn't work on Athena.
(setq lpr-command "lpr")

; lpr -d doesn't do anything in the Athena environment; use dvips to print
; DVI files.
(setq tex-dvi-print-command "dvips")

; Athena likes different X paste behavior
(setq mouse-yank-at-point t)

; Get info from the gnu locker as well as the normal places.
(setq Info-default-directory-list
      (append '("/afs/athena.mit.edu/project/gnu/info")
	      Info-default-directory-list))

(setq initial-major-mode '(lambda ()
			    (text-mode)
			    (auto-fill-mode 1)
			    (setq buffer-offer-save t)))

; Include PO server in rmail inbox list as well as obvious mbox files.
(setq local-inbox
      (cond ((file-accessible-directory-p "/var/mail/")
	     "/var/mail/$USER")
	    ((file-accessible-directory-p "/var/spool/mail/")
	     "/var/spool/mail/$USER")
	    ((file-accessible-directory-p "/usr/spool/mail/")
	     "/usr/spool/mail/$USER")))
(setq rmail-primary-inbox-list (list "~/mbox" local-inbox "po:$USER"))

(autoload 'discuss "discuss" "Emacs Discuss" t)

(autoload 'report-emacs-bug "emacsbug"
	  "Report a bug in Gnu emacs."
	  t)

(autoload 'clu-mode "clu-mode"
	  "Load CLU mode."
	  t)
(setq auto-mode-alist (append auto-mode-alist '(("\\.clu$" . clu-mode)
						("\\.equ$" . clu-mode))))

(autoload 'ispell-word "ispell"
	  "Check the spelling of a word in the buffer."
	  t)
(autoload 'ispell-region "ispell"
	  "Check the spelling of a region in the buffer."
	  t)
(autoload 'ispell-buffer "ispell"
	  "Check the spelling of the entire buffer."
	  t)

; Athena auto-save customizations

(defconst auto-save-main-directory
  (cond ((file-accessible-directory-p "/var/tmp/") "/var/tmp/")
	((file-accessible-directory-p "/tmp/") "/tmp/"))
  "The root of the auto-save directory; nil means use old style.")

; Put .saves files in same place as auto-save files.
(setq auto-save-list-file-prefix (concat auto-save-main-directory ".saves"))

(defun make-auto-save-file-name ()
  "Return file name to use for auto-saves of current buffer.
Does not consider auto-save-visited-file-name; that is checked
before calling this function.
You can redefine this for customization.
See also auto-save-file-name-p."
  (if auto-save-main-directory
      (if buffer-file-name
	  (concat auto-save-main-directory
		  "#"
		  (user-real-uid)
		  "."
		  (auto-save-replace-slashes buffer-file-name)
		  "#")
	(concat auto-save-main-directory
		"#%"
		(user-real-uid)
		"."
		(auto-save-replace-slashes (buffer-name))
		"#"))
    (if buffer-file-name
	(concat (file-name-directory buffer-file-name)
		"#"
		(file-name-nondirectory buffer-file-name)
		"#")
      (expand-file-name (concat "#%" (buffer-name) "#")))))

(defun auto-save-replace-slashes (name)
  "Replace all slashes in NAME with bangs."
  (let ((pos 0) (len (length name)))
    (setq ourname (make-string len ? ))
    (while (< pos len)
      (if (= (aref name pos) ?/)
	  (aset ourname pos ?@)
	(aset ourname pos (aref name pos)))
      (setq pos (+ 1 pos)))
    ourname))

