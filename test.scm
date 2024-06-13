(define bar 99)
(define foo bar)
(define x 10)
(define y 20)

(define (fak n)
  (if (<= n 1)
	1
	(* n (fak (- n 1)))))

(display "Testing simple recursion: ")
(display (fak 5))
(display endl)

(define (fib n)
 (if (<= n 2)
  1
  (+ (fib (- n 1))
     (fib (- n 2)))))

(display "Testing manual fibs:" endl)
(display (fib 1) endl)
(display (fib 2) endl)
(display (fib 3) endl)
(display (fib 4) endl)
(display (fib 5) endl)

(define (loop i n)
 (if (> i n)
  0
  (block
   (display "Fibonacci number " i " is " (fib i) endl)
   (loop (+ i 1) n))))

(display "Testing fibs in a loop" endl)
(loop 1 12)

(display "This is " true)

