;; GNUEmacs Macros used to make life easier in the postit program
;;
;; Author: Noah Mendelsohn, IBM T.J. Watson Research and MIT Project Athena
;;
;; Copyright (c) 1987 Massachusetts Institute of Technology
;;

;;
;;			forward-postit-entry
;;
(defun forward-postit-entry (n)
  "Move cursor to beginning of next postit entry field"
  (interactive "p")
  (if (< n 1) (setq n 1))
  (push-mark)
  (let ((i n))
    (while (> i 0)
      (search-forward "\n$")
      (if (looking-at "BeginEvent")
	  (search-forward "\n$")
      )
      (search-forward ":")
      (setq i (1- i))
    )
  )
)

;;
;;			backward-postit-entry
;;
(defun backward-postit-entry (n)
  "Move cursor to beginning of previous postit entry field"
  (interactive "p")
  (if (< n 1) (setq n 1))
  (setq postiterror nil)
  (push-mark)
  (let ((i n))
    (while (> i 0)
      (previous-line 1)
      (end-of-line)
      (search-backward "\n$")
      (if (looking-at "\n$BeginEvent")
	  (setq postiterror 't)
      )
      (if postiterror
	  (exchange-point-and-mark)
      )
      (if postiterror
	  (error "Already at first field")
      )
      (search-forward ":")
      (setq i (1- i))
    )
  )
)

(local-set-key "\e]" 'forward-postit-entry)
(local-set-key "\e[" 'backward-postit-entry)
(setq paragraph-start "^[#\|\$]")
