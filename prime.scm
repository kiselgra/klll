(define (prime n)
  (define (prime-loop i)
	(display "loop at " i endl)
	(define n-over-i (/ n i))
	(if (= n i) true
	    (if (= (* n-over-i i) n) ; this is n%i==0
		    false
			(prime-loop (+ i 1)))))
  (prime-loop 2))

(display (prime 107) endl)

