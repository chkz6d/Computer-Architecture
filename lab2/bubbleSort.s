  
addi $a0, $zero, 9
addi $a1 , $zero, 0
addi $a2 , $zero, 0
addi $t0, $zero, 5
addi $t1, $zero, 3
addi $t2, $zero, 6
addi $t3, $zero, 8
addi $t4, $zero, 9
addi $t5, $zero, 1
addi $t6, $zero, 4
addi $t7, $zero, 7
addi $t8, $zero, 2
addi $t9, $zero, 10
lui $v1, 4097($zero)
sw $t0, $zero($v1)
sw $t1, 4($v1)
sw $t2, 8($v1)
sw $t3, 12($v1)
sw $t4, 16($v1)
sw $t5, 20($v1)
sw $t6, 24($v1)
sw $t7, 28($v1)
sw $t8, 32($v1)
sw $t9, 36($v1)
sub $t0, $a0, $a1
blez $t0, 44
lw $t2, 0($v1)
addi $v1, $v1, 0x4
lw $t3, 0($v1)
sub $t4, $t2, $t3
addi $a1, $a1, 1
bgez $t4, 8
j 4194400
sw $t3, -4($v1)
sw $t2, 0($v1)
j 4194400
lui $v1, 4097($zero)
addi $a1, $zero, 0
sub $t1, $a0, $a2
addi $a2, $a2, 1
bgez $t1, -64
syscall