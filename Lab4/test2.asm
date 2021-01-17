.data
_Prompt: .asciiz "Enter an integer:  "
_ret: .asciiz "\n"
.globl main
.text
 li $t7,0x40
 jal main
 li $v0,10
 syscall
read:
  li $v0,4
  la $a0,_Prompt
  syscall
  li $v0,5
  syscall
  jr $ra
write:
  li $v0,1
  syscall
  li $v0,4
  la $a0,_ret
  syscall
  move $v0,$0
  jr $ra

fact:
  li $t3,1
  sw $t3, 16($sp)
  lw $t1,12($sp)
  lw $t2,16($sp)
  beq $t1,$t2,label2
  j label3
label2:
  lw $v0,12($sp)
  jr $ra
  j label1
label3:
  li $t3,1
  sw $t3, 16($sp)
  lw $t1,12($sp)
  lw $t2,16($sp)
  sub $t3,$t1,$t2
  sw $t3,20($sp)
  move $t0,$sp
  addi $sp,$sp,-32
  sw $ra,0($sp)
  lw $t1,20($t0)
  move $t3,$t1
  sw $t3,12($sp)
  jal fact
  lw $ra,0($sp)
  addi $sp,$sp,32
  sw $v0,24($sp)
  lw $t1,12($sp)
  lw $t2,24($sp)
  mult $t1,$t2
  mflo $t3
  sw $t3,28($sp)
  lw $v0,28($sp)
  jr $ra
label1:

main:
  addi $sp,$sp,-24
  addi $sp,$sp,-4
  sw $ra,0($sp)
  jal read
  lw $ra,0($sp)
  addi $sp,$sp,4
  sw $v0,20($sp)
  lw $t1,20($sp)
  move $t3,$t1
  sw $t3, 12($sp)
  li $t3,1
  sw $t3, 20($sp)
  lw $t1,12($sp)
  lw $t2,20($sp)
  bgt $t1,$t2,label7
  j label8
label7:
  move $t0,$sp
  addi $sp,$sp,-32
  sw $ra,0($sp)
  lw $t1,12($t0)
  move $t3,$t1
  sw $t3,12($sp)
  jal fact
  lw $ra,0($sp)
  addi $sp,$sp,32
  sw $v0,20($sp)
  lw $t1,20($sp)
  move $t3,$t1
  sw $t3, 16($sp)
  j label6
label8:
  li $t3,1
  sw $t3, 20($sp)
  lw $t1,20($sp)
  move $t3,$t1
  sw $t3, 16($sp)
label6:
  lw $a0, 16($sp)
  addi $sp, $sp, -4
  sw $ra,0($sp)
  jal write
  lw $ra,0($sp)
  addi $sp, $sp, 4
  li $t3,0
  sw $t3, 20($sp)
  lw $v0,20($sp)
  jr $ra
label4:
