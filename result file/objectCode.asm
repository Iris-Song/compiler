lui $sp,0x10040000
lui $fp,0x1003FFFC
j main
demo:
sw $ra 4($sp)
lw $s7 8($sp)
addi $s6 $zero 2
add $s7 $s7 $s6
addi $s5 $zero 2
mul $s4 $s7 $s5
add $v0 $zero $s4
lw $ra 4($sp)
jr $ra
main:
addi $s7 $zero 0
addi $s6 $zero 2
mul $s5 $s7 $s6
addi $s4 $zero 0
add $s3 $s4 $s5
addi $s5 $zero 4
mul $s3 $s3 $s5
addi $s2 $zero 3
addi $s1 $zero 0
addi $s0 $zero 2
mul $s2 $s1 $s0
addi $s3 $zero 1
add $s3 $s3 $s2
addi $s2 $zero 4
mul $s3 $s3 $s2
addi $s2 $zero 0
addi $s2 $zero 2
mul $s2 $s2 $s2
addi $s3 $zero 0
add $s3 $s3 $s2
addi $s2 $zero 4
mul $s3 $s3 $s2
lw $s2 0($sp)
addi $s2 $zero 1
add $s2 $s2 $s2
addi $s2 $zero 1
addi $s2 $zero 2
mul $s2 $s2 $s2
addi $s3 $zero 0
add $s3 $s3 $s2
addi $s2 $zero 4
mul $s3 $s3 $s2
addi $s2 $zero 0
addi $s2 $zero 4
mul $s2 $s2 $s2
addi $s2 $zero 1
addi $s2 $zero 4
mul $s2 $s2 $s2
lw $s2 0($sp)
lw $s2 0($sp)
add $s2 $s2 $s2
addi $s2 $zero 1
addi $s2 $zero 2
mul $s2 $s2 $s2
addi $s3 $zero 1
add $s3 $s3 $s2
addi $s2 $zero 4
mul $s3 $s3 $s2
addi $s2 $zero 0
addi $s2 $zero 2
mul $s2 $s2 $s2
addi $s3 $zero 0
add $s3 $s3 $s2
addi $s2 $zero 4
mul $s3 $s3 $s2
addi $s2 $zero 0
addi $s2 $zero 2
mul $s2 $s2 $s2
addi $s3 $zero 1
add $s3 $s3 $s2
addi $s2 $zero 4
mul $s3 $s3 $s2
addi $s2 $zero 1
addi $s2 $zero 2
mul $s2 $s2 $s2
addi $s3 $zero 0
add $s3 $s3 $s2
addi $s2 $zero 4
mul $s3 $s3 $s2
lw $s2 0($sp)
sw $s2 16($sp)
sw $sp 8($sp)
addi $sp $sp 8
jal demo
lw $sp 0($sp)
Label6:
lw $s7 0($sp)
sw $s7 16($sp)
lw $s7 0($sp)
sw $s7 20($sp)
sw $v0 24($sp)
sw $sp 8($sp)
addi $sp $sp 8
jal program
lw $sp 0($sp)
Label7:
j end
program:
sw $ra 4($sp)
addi $s7 $zero 0
lw $s6 12($sp)
lw $s5 16($sp)
add $s4 $s6 $s5
sw $s7 20($sp)
lw $s3 8($sp)
ble $s3 $s4 Label2
Label1:
lw $s7 12($sp)
lw $s6 16($sp)
mul $s7 $s7 $s6
addi $s6 $zero 1
add $s7 $s7 $s6
lw $s5 8($sp)
add $s5 $s5 $s7
sw $s5 24($sp)
j Label3
Label2:
lw $s7 8($sp)
sw $s7 24($sp)
Label3:
lw $s7 20($sp)
addi $s6 $zero 100
bgt $s7 $s6 Label5
Label4:
lw $s7 24($sp)
addi $s6 $zero 2
mul $s5 $s7 $s6
sw $s5 20($sp)
j Label3
Label5:
lw $v0 20($sp)
lw $ra 4($sp)
jr $ra
end:
