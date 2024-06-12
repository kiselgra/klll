(define bar 99)
(define foo bar)
(define x 10)
(define y 20)

(define (fak n)
  (if (<= n 1)
	1
	(* n (fak (- n 1)))))

(display (fak 5))

(define (fib n)
 (if (<= n 2)
  1
  (+ (fib (- n 1))
     (fib (- n 2)))))

(display (fib 1))
(display (fib 2))
(display (fib 3))
(display (fib 4))
(display (fib 5))

(define (loop i n)
 (if (> i n)
  0
  (block
   (display (fib i))
   (loop (+ i 1) n))))

(display 111111)

(display (loop 1 12))
