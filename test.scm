(define bar 99)
(define foo bar)
(define x 10)
(define y 20)

(define (fak n)
  (if (<= n 1)
	1
	(* n (fak (1- n)))))
