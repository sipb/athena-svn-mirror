; Eazel/theme.jl

;; Eazel sawfish theme
;;
;; A quick Eazel theme to be used for the demo stations at Linux World 2000.
;;
;; !!! This theme is not meant for public consumption !!!
;;
;; Design, Graphics, Concept, etc by: Arlo Rose
;; Implementation by: Seth Nickell
;;
;; © 2000 Eazel, Inc.


(let*
    ;; Update window title pixel length
  (
    (title-width
      (lambda (w)
	(let
	  ((w-width (car (window-dimensions w))))
	  (max 0 (min (- w-width 100) (text-width (window-name w)
            (get-font
             "-adobe-helvetica-bold-r-normal-*-*-120-*-*-p-*-iso8859-1")))))))

	;; ********* TITLE-BAR *********
	;; 6x22
        (top-left-corner-images (list (make-image "i-top-left-corner.png")
				       (make-image "top-left-corner.png")))

	;; 6x22
        (top-shaded-left-corner-images (list (make-image "i-s-top-left-corner.png")
				             (make-image "s-top-left-corner.png")))

	;; 24x22
	(top-title-text-images (list (make-image "i-top-title-text-repeat.png")
				     (make-image "top-title-text-repeat.png")))

	;; 24x22
	(top-teal-images (list (make-image "i-top-teal-repeat.png")
			       (make-image "top-teal-repeat.png")))

	;; 8x22
	(left-title-text-end-cap-images (list (make-image "i-left-title-text-end-cap.png")
					      (make-image "left-title-text-end-cap.png")))
	;; 19x22
	(right-title-text-end-cap-images (list (make-image "i-right-title-text-end-cap.png")
					       (make-image "right-title-text-end-cap.png")))

	;; 17x22
	(iconify-images (list (make-image "i-min.png")
			      (make-image "min.png") nil
			      (make-image "p-min.png")))

	;; 18x22
	(maximize-images (list (make-image "i-max.png")
			       (make-image "max.png") nil
			       (make-image "p-max.png")))

	;; 17x22
	(close-images (list (make-image "i-close.png")
			    (make-image "close.png") nil
			    (make-image "p-close.png")))

	;; 6x22
	(top-right-corner-images (list (make-image "i-top-right-corner.png")
				      (make-image "top-right-corner.png")))

	;; 6x22
	(top-shaded-right-corner-images (list (make-image "i-s-top-right-corner.png")
				              (make-image "s-top-right-corner.png")))

	;; *********** LEFT-BORDER ***********

	;; 6x40
	(left-teal-hangdown-images (list (make-image "i-left-teal-hangdown.png")
					 (make-image "left-teal-hangdown.png")))
	;; 6x16
	(left-repeat-images (list (make-image "i-left-repeat.png")
				  (make-image "left-repeat.png")))

	;; 6x20
	(left-teal-bottom-grow-images (list (make-image "i-left-teal-bottom-grow.png")
					    (make-image "left-teal-bottom-grow.png")))

	;; ********** RIGHT-BORDER ***********

	;; note that left-repeat-images is used on the right too

	;; 6x20
	(right-teal-bottom-grow-images (list (make-image "i-right-teal-bottom-grow.png")
					     (make-image "right-teal-bottom-grow.png")))


	;; ********** BOTTOM-BORDER **********

	;; 29x6
	(bottom-teal-left-grow-images (list (make-image "i-bottom-teal-left-grow.png")
					    (make-image "bottom-teal-left-grow.png")))

	;; 16x6
	(bottom-repeat-images (list (make-image "i-bottom-repeat.png")
				    (make-image "bottom-repeat.png")))

	;; 26x6
	(bottom-teal-right-grow-images (list (make-image "i-bottom-teal-right-grow.png")
					     (make-image "bottom-teal-right-grow.png")))

     (frame `(
	      ;; **TITLE-BAR**
              ;; top left corner
              ((background . ,top-left-corner-images)
	       (left-edge . -6)
	       (top-edge . -22)
	       (class . top-left-corner))

	      ;; teal stretch
	      ((background . ,top-teal-images)
	       (top-edge . -22)
               (left-edge . 0)
               (right-edge . 0)
	       ;;(right-edge . ,(lambda (w) (+ (title-width w) 79)))
	       (class . title))

	      ;; left text bumper
              ((background . ,left-title-text-end-cap-images)
	       (top-edge . -22)
               (right-edge . ,(lambda (w) (+ (title-width w) 71)))
               (class . title))

              ;; window title
              ((background . ,top-title-text-images)
	       (foreground . "black")
               (text . ,window-name)
               (x-justify . center)
	       (y-justify . center)          
	       (font . "-adobe-helvetica-bold-r-normal-*-*-120-*-*-p-*-iso8859-1")
               (top-edge . -22)
	       (right-edge . 71)
               (width . ,title-width)
               (class . title))

	      ;; right text bumper
	      ((background . ,right-title-text-end-cap-images)
	       (top-edge . -22)
	       (right-edge . 52)
	       (class . title))

	      ;; minimize button
              ((background . ,iconify-images)
               (right-edge . 35)
               (top-edge . -22)
               (class . iconify-button))

	      ;; maximize button
	      ((background . ,maximize-images)
	       (right-edge . 17)
	       (top-edge . -22)
	       (class . maximize-button))
	      
	      ;; close button
	      ((background . ,close-images)
	       (right-edge . 0)
	       (top-edge . -22)
	       (class . close-button))
	      
	      ;; top right corner
	      ((background . ,top-right-corner-images)		
	       (right-edge . -6)
	       (top-edge . -22)
	       (class . top-right-corner))
	      


	      
	      ;; **LEFT-BORDER**
	      ;; left teal hangdown
	      ((background . ,left-teal-hangdown-images)
	       (left-edge . -6)
	       (top-edge . 0)
	       (class . left-border))
	      
	      ;; left border stretch
	      ((background . ,left-repeat-images)
	       (left-edge . -6)
	       (top-edge . 40)
	       (bottom-edge . 20)
	       (class . left-border))
	      
	      ;; left teal bottom grow
	      ((background . ,left-teal-bottom-grow-images)
	       (left-edge . -6)
	       (bottom-edge . 0)
	       (class . bottom-left-corner))



	      
	      ;; **BOTTOM-BORDER**
	      ;; bottom teal left grow
	      ((background . ,bottom-teal-left-grow-images)
	       (left-edge . -6)
	       (bottom-edge . -6)
	       (class . bottom-left-corner))
	      
	      ;; bottom stretch
	      ((background . ,bottom-repeat-images)
	       (left-edge . 23)
	       (right-edge . 20)
	       (bottom-edge . -6)
	       (class . bottom-border))
	      
	      ;; bottom teal right grow
	      ((background . ,bottom-teal-right-grow-images)
	       (right-edge . -6)
	       (bottom-edge . -6)
	       (class . bottom-right-corner))




	      ;; **RIGHT-BORDER**
	      ;; right stretch
	      ((background . ,left-repeat-images)
	       (right-edge . -6)
	       (top-edge . 0)
	       (bottom-edge . 20)
	       (class . right-border))
	      
	      ;; right teal bottom grow
	      ((background . ,right-teal-bottom-grow-images)
	       (right-edge . -6)
	       (bottom-edge . 0)
	       (class . bottom-right-corner))))

       (shaped-frame `(
	      ;; **TITLE-BAR**
              ;; top left corner
              ((background . ,top-shaded-left-corner-images)
	       (left-edge . -6)
	       (top-edge . -22)
	       (class . top-left-corner))

	      ;; teal stretch
	      ((background . ,top-teal-images)
	       (top-edge . -22)
               (left-edge . 0)
	       (right-edge . ,(lambda (w) (+ (title-width w) 79)))
	       (class . title))

	      ;; left text bumper
              ((background . ,left-title-text-end-cap-images)
	       (top-edge . -22)
               (right-edge . ,(lambda (w) (+ (title-width w) 71)))
               (class . title))

              ;; window title
              ((background . ,top-title-text-images)
	       (foreground . "black")
	       (font . "-adobe-helvetica-bold-r-normal-*-*-120-*-*-p-*-iso8859-1")
               (text . ,window-name)
               (x-justify . center)
	       (y-justify . center)
               (top-edge . -22)
	       (right-edge . 71)
               (width . ,title-width)
               (class . title))

	      ;; right text bumper
	      ((background . ,right-title-text-end-cap-images)
	       (top-edge . -22)
	       (right-edge . 52)
	       (class . title))

	      ;; minimize button
              ((background . ,iconify-images)
               (right-edge . 35)
               (top-edge . -22)
               (class . iconify-button))

	      ;; maximize button
	      ((background . ,maximize-images)
	       (right-edge . 17)
	       (top-edge . -22)
	       (class . maximize-button))
	      
	      ;; close button
	      ((background . ,close-images)
	       (right-edge . 0)
	       (top-edge . -22)
	       (class . close-button))
	      
	      ;; top right corner
	      ((background . ,top-shaded-right-corner-images)		
	       (right-edge . -6)
	       (top-edge . -22)
	       (class . top-right-corner)))))

  (add-frame-style 'Eazel
		   (lambda (w type)
		     (case type
		       ((default) frame)
		       ((transient) frame)
		       ((shaped) shaped-frame)
		       ((shaped-transient) shaped-frame))))
  (call-after-property-changed
   'WM_NAME (lambda ()
              (rebuild-frames-with-style 'Eazel))))
