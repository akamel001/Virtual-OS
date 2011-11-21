loadi  0 0   ! i = 0
compri 0 6   ! 6 pairs to read
jumpe  9     ! i == 6 done
read   1
read   2
add    1 2
write  1
addi   0 1   ! i++
jump   1     ! loop again
halt
