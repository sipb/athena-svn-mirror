;;; Spelling correction interface for GNU EMACS using "ispell"

;;; Walt Buehring
;;; Texas Instruments - Computer Science Center
;;; ARPA:  Buehring%TI-CSL@CSNet-Relay
;;; UUCP:  {smu, texsun, im4u, rice} ! ti-csl ! buehring

;;; ispell-region and associate routines added by
;;; Perry Smith
;;; pedz@bobkat
;;; Tue Jan 13 20:18:02 CST 1987

;;; To fully install this, add this file to your GNU lisp directory and 
;;; compile it with M-X byte-compile-file.  Then add the following to the
;;; appropriate init file:

;;;  (autoload 'ispell-word "ispell"
;;;    "Check the spelling of word in buffer." t)
;;;  (global-set-key "\e$" 'ispell-word)

;;; If run on a heavily loaded system, the timeout value in ispell-check 
;;; and the initial sleep time in ispell-init-process may need to be increased.

;;; No warranty expressed or implied.  All sales final.  Void where prohibited.
;;; If you don't like it, change it.

(defvar ispell-syntax-table nil)

(if (null ispell-syntax-table)
    ;; The following assumes that the standard-syntax-table
    ;; is static.  If you add words with funky characters
    ;; to your dictionary, the following may have to change.
    (progn
      (setq ispell-syntax-table (make-syntax-table))
      ;; Make certain characters word constituents
      ;; (modify-syntax-entry ?' "w   " ispell-syntax-table)
      ;; (modify-syntax-entry ?- "w   " ispell-syntax-table)
      ;; Get rid on existing word syntax on certain characters 
      (modify-syntax-entry ?0 ".   " ispell-syntax-table)
      (modify-syntax-entry ?1 ".   " ispell-syntax-table)
      (modify-syntax-entry ?2 ".   " ispell-syntax-table)
      (modify-syntax-entry ?3 ".   " ispell-syntax-table)
      (modify-syntax-entry ?4 ".   " ispell-syntax-table)
      (modify-syntax-entry ?5 ".   " ispell-syntax-table)
      (modify-syntax-entry ?6 ".   " ispell-syntax-table)
      (modify-syntax-entry ?7 ".   " ispell-syntax-table)
      (modify-syntax-entry ?8 ".   " ispell-syntax-table)
      (modify-syntax-entry ?9 ".   " ispell-syntax-table)
      (modify-syntax-entry ?$ ".   " ispell-syntax-table)
      (modify-syntax-entry ?% ".   " ispell-syntax-table)))


(defun ispell-word (&optional quietly)
  "Check spelling of word at or before dot.
If word not found in dictionary, display possible corrections in a window 
and let user select."
  (interactive)
  (let* ((current-syntax (syntax-table))
	 start end word poss replace)
    (unwind-protect
	(save-excursion
	  ;; Ensure syntax table is reasonable 
	  (set-syntax-table ispell-syntax-table)
	  ;; Move backward for word if not already on one.
	  (if (not (looking-at "\\w"))
	      (re-search-backward "\\w" (dot-min) 'stay))
	  ;; Move to start of word
	  (re-search-backward "\\W" (dot-min) 'stay)
	  ;; Find start and end of word
	  (or (re-search-forward "\\w+" nil t)
	      (error "No word to check."))
	  (setq start (match-beginning 0)
		end (match-end 0)
		word (buffer-substring start end)))
      (set-syntax-table current-syntax))
    (or quietly (message "Checking spelling of %s..." (upcase word)))
    (setq poss (ispell-check word))
    (cond ((eq poss t)
	   (or quietly (message "Found %s" (upcase word))))
	  ((stringp poss)
	   (or quietly (message "Found it because of %s" (upcase poss))))
	  ((null poss)
	   (or quietly (message "Could Not Find %s" (upcase word))))
	  (t (setq replace (ispell-choose poss word))
	     (if replace
		 (progn
		    (goto-char end)
		    (delete-region start end)
		    (insert-string replace)))))
    poss))


(defun ispell-choose (choices word)
  "Display possible corrections from list CHOICES.  Return chosen word
if one is chosen; Return nil to keep word"
  (unwind-protect 
      (save-window-excursion
	(let ((count 0)
	      (words choices)
	      (window-min-height 2)
	      char num result)
	  (overlay-window 3)
	  (switch-to-buffer "*Choices*") (erase-buffer)
	  (setq mode-line-format "--  %b  --")
	  (while words
	    (if (> (+ 7 (current-column) (length (car words))) (window-width))
		(insert "\n"))
	    (insert "(" (+ count ?1) ") " (car words) "  ")
	    (setq words (cdr words)
		  count (1+ count)))
	  (select-window (next-window))
	  (while (eq t
		     (setq result
			   (progn
			     (message "Enter letter to replace word;  Space to flush")
			     (setq char (upcase (read-char)))
			     (setq num (- char ?1))
			     (cond ((= char ? ) nil)
				   ((= char ?I)
				    (ispell-check (concat "*" word))
				    nil)
				   ((= char ?A)
				    (ispell-check (concat "@" word))
				    nil)
				   ((= char ?R) (read-string "Replacement: " nil))
				   ((and (>= num 0) (< num count)) (nth num choices))
				   (t (ding) t))))))
	  result))
    ;; Protected forms...
    (bury-buffer "*Choices*")))


(defun overlay-window (height)
  "Create a (usually small) window with HEIGHT lines and avoid
recentering."
  (save-excursion
    (let ((oldot (save-excursion (beginning-of-line) (dot)))
	  (top (save-excursion (move-to-window-line height) (dot)))
	  newin)
      (if (< oldot top) (setq top oldot))
      (setq newin (split-window-vertically height))
      (set-window-start newin top))))


(defvar ispell-process nil
  "Holds the process object for 'ispell'")

;;; create signal used by ispell-filter and ispell-check
(put 'ispell-output 'error-conditions '(ispell-output))

(defun ispell-check (word)
  "Check spelling of string WORD, return either t for an exact match, a string
containing the root word for a match via suffix removal, a list of possible 
correct spellings, or nil for a complete miss."
  (let ((debug-on-error t)) ; defeat 19.29+ process filter exception handling
    (ispell-init-process)
    (send-string ispell-process (concat word "\n"))
    (condition-case output
	(progn
	  (sleep-for 20)
	  (error "Timeout waiting for ispell process output"))
      (ispell-output (ispell-parse-output (car (cdr output)))))))

(defun ispell-parse-output (output)
"Parse the OUTPUT string of 'ispell' and return a value as specified by the 
'ispell-check' function."
  (cond
   ((string= output "*") t)
   ((string= output "#") nil)
   ((string= (substring output 0 1) "+")
    (substring output 2))
   (t
    (let ((choice-list '()))
      (while (not (string= output ""))
	(let* ((start (string-match "[A-z]" output))
	       (end (string-match " \\|$" output start)))
	  (if start
	      (setq choice-list (cons (substring output start end)
				      choice-list)))
	  (setq output (substring output (1+ end)))))
      choice-list))))


(defvar ispell-process-output ""
  "Holds partial output from the 'ispell' process")

(defun ispell-filter (process output)
  "The filter-function for 'ispell'.  Signals complete line using the 
ispell-output signal"
  (if (string= "\n" (substring output (1- (length output))))
      (progn
	(setq output (concat ispell-process-output
			     (substring output 0 (1- (length output))))
	      ispell-process-output "")
	(signal 'ispell-output (list output)))
      (setq ispell-process-output (concat ispell-process-output output))))

(defun ispell-init-process ()
  "Check status of 'ispell' process and start if necessary; set up 
filter function for output."
  (if (or (not ispell-process)
	  (not (eq (process-status ispell-process) 'run)))
      (progn
	(message "Starting new ispell process...")
	(and (get-buffer "*ispell*") (kill-buffer "*ispell*"))
	(setq ispell-process (start-process "ispell" "*ispell*"
					   "ispell" "-a"))
	(set-process-filter ispell-process 'ispell-filter)
	(process-kill-without-query ispell-process)
	(sit-for 3))))

(defvar ispell-filter-hook "/bin/cat"
  "Filter to pass a region through before sending it to ispell.
Typically this is set to cat, deroff, detex, etc.")
(make-variable-buffer-local 'ispell-filter-hook)

(defvar ispell-filter-hook-args nil
  "Arguments to pass to ispell-filter-hook")
(make-variable-buffer-local 'ispell-filter-hook-args)

; This routine has certain limitations brought about by the filter
; hook.  For example, deroff will take ``\fBcat\fR'' and spit out
; ``cat''.  This is hard to search for since word-search-forward will
; not match at all and search-forward for ``cat'' will match
; ``concatinate'' if it happens to occur before.  I attempt to
; minimize these problems by always searching for each word in the
; original buffer even if it is not misspelled.  This slows things
; down.

(defun ispell-region (start end)
  "Check a region for spelling errors interactively.  The variable
which should be buffer or mode specific ispell-filter-hook is called
to filter out text processing commands."
  (interactive "r")
  (let ((this-buf (current-buffer))
	(spell-buf (get-buffer-create "ispell-temp"))
	(current-syntax (syntax-table))
	word poss replace word-start word-end)
    (unwind-protect
	(save-excursion
	  (set-buffer spell-buf)
	  (erase-buffer)
	  (set-buffer this-buf)
	  (if ispell-filter-hook-args
	      (call-process-region start end ispell-filter-hook nil
				   spell-buf nil ispell-filter-hook-args)
	    (call-process-region start end ispell-filter-hook nil
				 spell-buf nil))
	  (goto-char start)
	  (set-buffer spell-buf)
	  (set-syntax-table ispell-syntax-table)
	  (goto-char (point-min))
	  (while (progn
		   (message "Looking for a misspelled word")
		   (re-search-forward "\\W*\\(\\w+\\)" nil t))
	    (setq word (buffer-substring (setq word-start (match-beginning 1))
					 (setq word-end (match-end 1))))
	    (setq poss (ispell-check word))
	    (set-buffer this-buf)
	    (or (search-forward word nil t)
		(error "Can not find %s in original text" word))
	    (if (not (or (eq poss t) (stringp poss))) ;bad word
		(progn
		  (sit-for 0)
		  (setq replace (ispell-choose poss word))
		  (if replace
		      (replace-match replace))))
	    (set-buffer spell-buf))
	  (message "Done"))
      (set-syntax-table current-syntax))))

(defun ispell-buffer ()
  "Pass the entire buffer through ispell"
  (interactive)
  (ispell-region (point-min) (point-max)))
