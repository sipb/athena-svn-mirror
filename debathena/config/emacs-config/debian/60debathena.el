; mh-rmail won't find mh programs by default; tell it where they are.
(setq mh-progs "/usr/bin/"
      mh-lib-progs "/usr/lib/debathena-nmh/"
      mh-lib "/etc/nmh/")

; Security measure.
(setq inhibit-local-variables t)

; Too many users get bit if we don't set this.
(setq require-final-newline t)

; Handle bug reports locally.
(setq bug-gnu-emacs "bugs@mit.edu")

; Force outgoing mail domain to be "mit.edu" instead of local machine name.
(setq mail-host-address "mit.edu")

; lpr -d doesn't do anything in the Athena environment; use dvips to print
; DVI files.
(setq tex-dvi-print-command "dvips")

; Athena likes different X paste behavior
(setq mouse-yank-at-point t)

; Cosmetic change, preferred by most users as far as we know.  (And
; consistent with gnome-terminal, xterm, etc.)
(blink-cursor-mode 0)

; Change the initial scratch buffer to avoid people losing text they
; erroneously type into it.  Also make it a text buffer instead of a
; lisp buffer.
(setq initial-major-mode '(lambda ()
			    (text-mode)
			    (auto-fill-mode 1)
			    (setq buffer-offer-save t)))
; compensate for the above
(setq initial-scratch-message "\
This buffer is for notes you don't want to save.  If you want to create
a file, visit that file with C-x C-f, then enter the text in that file's
own buffer.

")

; Some gnus settings.  We set nnmail-crosspost-link-function to
; 'copy-file because AFS does not support hard links.
(setq gnus-default-nntp-server "news.mit.edu"
      gnus-local-organization "Massachusetts Institute of Technology"
      nnmail-crosspost-link-function 'copy-file)

; Athena auto-save customizations

(defconst auto-save-main-directory "/var/tmp/"
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
		  (int-to-string (user-real-uid))
		  "."
		  (auto-save-replace-slashes buffer-file-name)
		  "#")
	(concat auto-save-main-directory
		"#%"
		(int-to-string (user-real-uid))
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
