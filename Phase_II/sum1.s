loadi  0 1   ! i = 1
loadi  1 0   ! sum = 0
read   2
compr  0 2
jumpe  8     ! done
add    1 0   ! sum += i
addi   0 1   ! i++
jump   3     ! loop again
write  1
halt
