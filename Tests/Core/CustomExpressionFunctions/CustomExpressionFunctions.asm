.gba
.create "output.bin",0

.macro check,exp,value
  .if (exp) != value
    .error "Error: " + °(exp) + " = " + (exp) + " != " + value
  .endif
.endmacro

; simple function with multiple parameters
.expfunc lerp(a, b, t), a + t*(b-a)

check lerp(1, 5, 0.75), 4

; resursive function
.expfunc fib(n), n <= 0 ? 0 : n <= 2 ? 1 : fib(n-1)+fib(n-2)

check fib(0), 0
check fib(2), 1
check fib(8), 21

; function that depends on architecture specific functions. Change the architecture afterwards to check
; if it has any influence
.expfunc isArmWrapper(), isArm()

check isArmWrapper(), 0

.psp

.close
