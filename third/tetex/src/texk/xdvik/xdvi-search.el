;;; (X)Emacs frontend to forward search with xdvi(k).
;;; Requires xdvi(k) >= 22.38.
;;; See the section on FORWARD SEARCH in the xdvi man page for more
;;; information on forward and reverse search.
;;;
;;; This file is available from: http://xdvi.sourceforge.net/xdvi-search.el
;;;
;;; Preliminary test version, tested with Emacs 20.4 to 21.2 and Xemacs 21.1
;;;
;;; This program is free software; you can redistribute it and/or
;;; modify it under the terms of the GNU General Public License
;;; as published by the Free Software Foundation; either version 2
;;; of the License, or (at your option) any later version.
;;; 
;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;; 
;;; You should have received a copy of the GNU General Public License
;;; along with this program; if not, write to the Free Software
;;; Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
;;;
;;;
;;; Usage:
;;;
;;; - Add this file to some place where emacs can find it
;;;   (e.g. put it into a directory ~/emacs/, which you add to your
;;;   load path by putting the following line into your .emacs file:
;;;   
;;;      (add-to-list 'load-path (expand-file-name "~/emacs/"))
;;;   
;;;   Then, add the following line to your .emacs file:
;;;   
;;;      (require 'xdvi-search)
;;;   
;;;   After compiling your .tex file with source specials activated, you should
;;;   be able to use
;;;      M-x xdvi-jump-to-line
;;;   to make xdvi jump to the current location of the cursor.
;;;   
;;;   You could also bind this command to a key, e.g. by adding
;;;   a binding to tex-mode-hook:
;;;   
;;;      (add-hook 'tex-mode-hook (lambda ()
;;;                                 (local-set-key "\C-c\C-j" 'xdvi-jump-to-line)))
;;;
;;;   (without AucTeX; use LaTeX-mode-hook instead of tex-mode-hook with AucTeX.
;;;   Note that with AucTeX, C-c C-j is already bound to LaTeX-insert-item, so
;;;   you might want to use a different key combination instead.)
;;;
;;;
;;; Please send bug reports, improvements etc. to
;;; <stefanulrich@users.sourceforge.net>.
;;;


(defgroup xdvi-search nil
      "Support for inverse search with (La)TeX and DVI previewers."
      :group 'languages)

(defcustom explicit-shell-file-name nil
  "*If non-nil, file name to use for explicitly requested inferior shell."
  :type '(choice (const :tag "nil" nil) file)
  :group 'xdvi-search)


(defcustom xdvi-bin "xdvi"
  "*Name of the xdvi executable."
  :type '(choice (const "xdvi") file)
  :group 'xdvi-search)

(defcustom xdvi-logfile "~/.xdvi-log"
  "*Write xdvi output to this file, or throw away output if set to nil."
  :type '(choice (const :tag "nil" nil) file)
  :group 'xdvi-search)

(defun xdvi-jump-to-line-fullpath ()
  (interactive)
  "See `xdvi-jump-to-line' for documentation."
  (xdvi-jump-to-line 3))

(defun xdvi-jump-to-line-relpath ()
  (interactive)
  "See `xdvi-jump-to-line' for documentation."
  (xdvi-jump-to-line 2))

(defun xdvi-jump-to-line (&optional prefix)
  "Call xdvi to perform a `forward search' for current file and line number.
Xdvi needs three pieces of information for this:

- the current line number
- the name of the current input file
- the name of the master .tex file.

The master .tex file (we follow AucTeX's terminology here) is the same
as `\jobname' and determines the name of the DVI file.  It is either
obtained by using AucTeX's variable `TeX-master-file' or, in case
AucTeX is available, by calling `xdvi-master-file-name', a function
which tries to mimick AucTeX's behaviour to obtain this filename.

The line number and current input file need to be given in the same
way as they are contained in the source specials in the DVI file. While
this is straightforward for the line number, it's a bit more complicated
for the current input file, since there are so many ways of specifying
an input file for (La)TeX, which will all lead to different information
in the source specials.

For example, assume that the parent file is in path
`/home/user/tex/diss/' and the file `chapter1' is in the subdirectory
`chapters/'; then the following three variants are all valid ways
of including `chapter1':

   (1) Filename only: \\input{chapter1}
   (2) Relative path name: \\input{chapters/chapter1}
   (3) Full path name: \\input{/home/user/tex/diss/chapters/chapter1}

\(The example uses `\\input', but the situation would be similar for
`\\include').

Since the parent .tex file would need to be parsed to find out which
of the variants has actually been used (and I'd like to avoid that),
`xdvi-jump-to-line' comes in three variants:

- `xdvi-jump-to-line' for variant (1) above;
- `xdvi-jump-to-line-relpath' for variant (2), and
- `xdvi-jump-to-line-fullpath' for variant (3).

The first one is the most straightforward one, and the `recommended'
way of using input files. The second command tries to guess the path
component by using the difference between the current path and the
path of the master .tex file, and the third command uses the full path
to the current file. You'll need to use whichever command matches your
favorite way of specifying additional input files.

Actually the `fullpath' and `relpath' variants just invoke
`xdvi-jump-to-line' with various prefix arguments, and one could
obtain the same effect by using `xdvi-jump-to-line' interactively with
prefix argument 3 (full path) or 2 (relative path). Without a prefix
argument or with a prefix argument of 1, the default (filename only)
is used."
  (interactive "P")
  (save-excursion
    (save-restriction
      (widen)
      (beginning-of-line 1)
      (if (not prefix)
	  (setq prefix 1))
      (let* (;;; current line in file.
	     ;;; count-lines yields different results at beginning and in the
             ;;; middle of a line, so we normalize by going to BOL first and
             ;;; then adding 1
	     (curr-line (+ 1 (count-lines (point-min) (point))))
	     ;;; name of master .tex file, to be used as .dvi basename:
	     (master-file (expand-file-name (if (fboundp 'TeX-master-file)
					     (TeX-master-file t)
					   (xdvi-get-masterfile (xdvi-master-file-name)))))
	     ;;; .dvi file name:
	     (dvi-file (concat (file-name-sans-extension master-file) ".dvi"))
	     ;;; current source file name, depending on how much we want of the path:
	     (filename
	      (if (> prefix 1)
		  (if (> prefix 2)
		      ;;; full path name, for use with TeX patch and when using the
		      ;;; full path in \input or \include with srcltx.sty:
		      (expand-file-name (buffer-file-name))
		    ;;; relative path name: if current path and path of master file match partly,
		    ;;; use the rest of this path; else use buffer name. This can treat both the
		    ;;; cases when a relative path is used in \input{}, or when just the filename
		    ;;; is used and the file is located in the current directory (suggested by
		    ;;; frisk@isy.liu.se).
		    (if (string-match (concat "^" (regexp-quote (file-name-directory master-file)))
				      (buffer-file-name))
			(substring (buffer-file-name) (match-end 0))
		      (buffer-file-name)))
		;;; buffer file name without path:
		(file-name-nondirectory (buffer-file-name)))))
	(if (not (file-exists-p dvi-file))
	    (message "File %s does not exist; you need to run latex first!" dvi-file)
	    (save-window-excursion
	      ;;; obtain the type of I/O-redirection needed for current shell.
	      (let* ((default-shell
		       (or (and (boundp 'explicit-shell-file-name) explicit-shell-file-name)
			   (getenv "ESHELL")
			   (getenv "SHELL")
			   "/bin/sh"))
		     (shell-redirection
		      (cond ((string-match "/bin/t?csh" default-shell)
			     (list ">&" ""))
			    ;; bash/ksh/sh
			    ((list ">" "2>&1")))))
	        ;;; The reason for using `shell' instead of
	        ;;; `start-process' or `call-process' is that I haven't
	        ;;; found a way of viewing the stderr/stdout output of the
	        ;;; xdvi child process without having either xdvi
	        ;;; freezing, Emacs freezing, or Emacs killing the xdvi
	        ;;; child process (which is forked when running with
	        ;;; -sourceposition).  Making xdvi disassociate from the
	        ;;; controling terminal is clearly not an option either ...
		(cond ((string-match "XEmacs" emacs-version)
		       (shell))
		      ((< emacs-major-version 21)
		       (shell))
		      ((shell "*xdvi-shell*")))
		(comint-send-input)
		(insert xdvi-bin
			" -sourceposition '" (int-to-string curr-line) " " filename "' "
			dvi-file
			" "
			(car shell-redirection)
			" "
		      ;;; we could probably do without the logfile, but it's an easy
		      ;;; way for toggling between /dev/null and logging.
			(if xdvi-logfile
			    (let ((xdvi-logfile-fullname (expand-file-name xdvi-logfile)))
			      ;;; to avoid problems with `noclobber'
			      (if (file-exists-p xdvi-logfile-fullname)
				  (delete-file xdvi-logfile-fullname))
			      xdvi-logfile-fullname)
			  "/dev/null")
			" "
			(car (cdr shell-redirection))
			" &")
		(if xdvi-logfile
		    (message "Writing xdvi output to \"%s\"." xdvi-logfile))
		(comint-send-input))))))))


(defun xdvi-get-masterfile (file)
  "Small helper function for AucTeX compatibility.
Converts the special value t that TeX-master might be set to
into a real file name."
  (if (eq file t)
      (buffer-file-name)
    file))


(defun xdvi-master-file-name ()
  "Emulate AucTeX's TeX-master-file function.
Partly copied from tex.el's TeX-master-file and TeX-add-local-master."
  (if (boundp 'TeX-master)
      TeX-master
    (let ((master-file (read-file-name "Master file (default this file): ")))
      (if (y-or-n-p "Save info as local variable? ")
	  (progn
	    (goto-char (point-max))
	    (if (re-search-backward "^\\([^\n]+\\)Local Variables:" nil t)
		(let* ((prefix (if (match-beginning 1)
				   (buffer-substring (match-beginning 1) (match-end 1))
				 ""))
		      (start (point)))
		  (re-search-forward (regexp-quote (concat prefix "End:")) nil t)
		  (if (re-search-backward (regexp-quote (concat prefix "TeX-master")) start t)
		      ;;; if TeX-master line exists already, replace it
		      (progn
			(beginning-of-line 1)
			(kill-line 1))
		    (beginning-of-line 1))
		  (insert prefix "TeX-master: " (prin1-to-string master-file) "\n"))
	      (insert "\n%%% Local Variables: "
;;; mode is of little use without AucTeX ...
;;;		      "\n%%% mode: " (substring (symbol-name major-mode) 0 -5)
		      "\n%%% TeX-master: " (prin1-to-string master-file)
		      "\n%%% End: \n"))
	    (save-buffer)
	    (message "(local variables written.)"))
	(message "(nothing written.)"))
      (set (make-local-variable 'TeX-master) master-file))))


(provide 'xdvi-search)

;;; page break to avoid "Local variables entry is missing the prefix" error for previous stuff


;;; xdvi-search.el ends here
