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

min:
  lw $t1,12($sp)
  lw $t2,16($sp)
  blt $t1,$t2,label3
  j label2
label3:
  lw $v0,12($sp)
  jr $ra
label2:
  lw $v0,16($sp)
  jr $ra
label1:

main:
  addi $sp,$sp,-32
  li $t3,5
  sw $t3, 24($sp)
  lw $t1,24($sp)
  move $t3,$t1
  sw $t3, 20($sp)
  li $t3,1
  sw $t3, 32($sp)
  lw $t1,32($sp)
  move $t3,$t1
  sw $t3, 28($sp)
  addi $sp,$sp,-4
  sw $ra,0($sp)
  jal read
  lw $ra,0($sp)
  addi $sp,$sp,4
  sw $v0,36($sp)
  lw $t1,36($sp)
  move $t3,$t1
  sw $t3, 12($sp)
  addi $sp,$sp,-4
  sw $ra,0($sp)
  jal read
  lw $ra,0($sp)
  addi $sp,$sp,4
  sw $v0,36($sp)
  lw $t1,36($sp)
  move $t3,$t1
  sw $t3, 16($sp)
  move $t0,$sp
  addi $sp,$sp,-20
  sw $ra,0($sp)
  lw $t1,16($t0)
  move $t3,$t1
  sw $t3,12($sp)
  lw $t1,20($t0)
  move $t3,$t1
  sw $t3,16($sp)
  jal min
  lw $ra,0($sp)
  addi $sp,$sp,20
  sw $v0,36($sp)
  lw $t1,36($sp)
  move $t3,$t1
  sw $t3, 16($sp)
  li $t3,0
  sw $t3, 36($sp)
  lw $t1,36($sp)
  move $t3,$t1
  sw $t3, 0($sp)
label12:
  lw $t1,28($sp)
  lw $t2,12($sp)
  ble $t1,$t2,label11
  j label9
label11:
  li $t3,10
  sw $t3, 36($sp)
  lw $t1,28($sp)
  lw $t2,36($sp)
  blt $t1,$t2,label10
  j label9
label10:
  lw $t1,28($sp)
  lw $t2,16($sp)
  beq $t1,$t2,label14
  j label13
label14:
 li $t1,0
  lw $t2,28($sp)
 sub $t3,$t1,$t2
  sw $t3,36($sp)
  lw $t1,0($sp)
  lw $t2,36($sp)
  add $t3,$t1,$t2
  sw $t3,0($sp)
  li $t3,1
  sw $t3, 36($sp)
  lw $t1,28($sp)
  lw $t2,36($sp)
  add $t3,$t1,$t2
  sw $t3,28($sp)
  j label12
label13:
  lw $t1,0($sp)
  lw $t2,28($sp)
  add $t3,$t1,$t2
  sw $t3,0($sp)
  lw $t1,28($sp)
  addi $t1,$t1,1
  sw $t1,28($sp)
  j label12
label9:
  li $t3,1
  sw $t3, 36($sp)
  lw $t1,36($sp)
  li $t2,0
  beq $t1,$t2,label19
  li $t3,0
  sw $t3, 40($sp)
  lw $t1,40($sp)
  li $t2,0
  beq $t1,$t2,label19
  li $t3,1
  sw $t3, 36($sp)
  j label20
label19:
  li $t3,0
  sw $t3, 36($sp)
label20:
  lw $t1,36($sp)
  move $t3,$t1
  sw $t3, 16($sp)
  lw $a0, 0($sp)
  addi $sp, $sp, -4
  sw $ra,0($sp)
  jal write
  lw $ra,0($sp)
  addi $sp, $sp, 4
  lw $a0, 16($sp)
  addi $sp, $sp, -4
  sw $ra,0($sp)
  jal write
  lw $ra,0($sp)
  addi $sp, $sp, 4
  li $t3,0
  sw $t3, 36($sp)
  lw $v0,36($sp)
  jr $ra
label4:
