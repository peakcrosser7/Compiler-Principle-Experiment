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

main:
  addi $sp,$sp,-52
  li $t3,0
  sw $t3, 16($sp)
  lw $t1,16($sp)
  move $t3,$t1
  sw $t3, 12($sp)
  li $t3,1
  sw $t3, 24($sp)
  lw $t1,24($sp)
  move $t3,$t1
  sw $t3, 20($sp)
  li $t3,0
  sw $t3, 32($sp)
  lw $t1,32($sp)
  move $t3,$t1
  sw $t3, 28($sp)
  addi $sp,$sp,-4
  sw $ra,0($sp)
  jal read
  lw $ra,0($sp)
  addi $sp,$sp,4
  sw $v0,40($sp)
  lw $t1,40($sp)
  move $t3,$t1
  sw $t3, 36($sp)
label5:
  lw $t1,28($sp)
  lw $t2,36($sp)
  blt $t1,$t2,label4
  j label3
label4:
  lw $t1,12($sp)
  lw $t2,20($sp)
  add $t3,$t1,$t2
  sw $t3,44($sp)
  lw $t1,44($sp)
  move $t3,$t1
  sw $t3, 40($sp)
  lw $a0, 20($sp)
  addi $sp, $sp, -4
  sw $ra,0($sp)
  jal write
  lw $ra,0($sp)
  addi $sp, $sp, 4
  lw $t1,20($sp)
  move $t3,$t1
  sw $t3, 12($sp)
  lw $t1,40($sp)
  move $t3,$t1
  sw $t3, 20($sp)
  lw $t1,28($sp)
  addi $t1,$t1,1
  sw $t1,28($sp)
  j label5
label3:
  li $t3,0
  sw $t3, 40($sp)
  lw $v0,40($sp)
  jr $ra
label1:
