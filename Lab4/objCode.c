/* 目标代码生成 */
#include "def.h"

//目标代码生成：朴素寄存器分配法将中间代码转换成MISPS代码
void objectCode(struct CodeNode *head) {
    //初始化代码
    //数据段
    printf(".data\n");
    printf("_Prompt: .asciiz \"Enter an integer:  \"\n");
    printf("_ret: .asciiz \"\\n\"\n");
    printf(".globl main\n");
    //代码段
    printf(".text\n");
    printf(" li $t7,0x40\n");
    printf(" jal main\n");
    printf(" li $v0,10\n");
    printf(" syscall\n");
    //read函数代码
    printf("read:\n");
    printf("  li $v0,4\n");
    printf("  la $a0,_Prompt\n");
    printf("  syscall\n");
    printf("  li $v0,5\n");     //read int
    printf("  syscall\n");
    printf("  jr $ra\n");
    //write函数代码
    printf("write:\n");
    printf("  li $v0,1\n");
    printf("  syscall\n");
    printf("  li $v0,4\n");
    printf("  la $a0,_ret\n");
    printf("  syscall\n");
    printf("  move $v0,$0\n");
    printf("  jr $ra\n");

    if (head == NULL) {
        printf("No Object Code!\n");
        return;
    }

    struct CodeNode *h = head, *p;
    int i, j, idx, type;
    /* 
    MIPS说明：
    寄存器：$t1, $t2-操作数1、2；$t3-运算结果，$ra-函数返回地址，$sp栈顶指针
           $v0-返回值
    */
    do {
        switch (h->op) {
        case ASSIGNOP:
            if (h->opn1.kind == INT)
                //加载立即数（操作数）到$t3
                printf("  li $t3,%d\n", h->opn1.const_int);
            else {
                //加载操作数到$t1再到$t3
                printf("  lw $t1,%d($sp)\n", h->opn1.offset);
                printf("  move $t3,$t1\n");
            }
            //从$t3存运算结果
            printf("  sw $t3, %d($sp)\n", h->result.offset);
            break;
        case PLUS:
        case MINUS:
        case STAR:
        case DIV:
            //操作数1
            if (h->opn1.kind == ID)
                printf("  lw $t1,%d($sp)\n", h->opn1.offset);
            else
                printf("  li $t1,%d\n", h->opn2.const_int);
            //操作数2
            if (h->opn2.kind == ID)
                printf("  lw $t2,%d($sp)\n", h->opn2.offset);
            else
                printf("  li $t2,%d\n", h->opn2.const_int);
            //运算符
            if (h->op == PLUS)
                printf("  add $t3,$t1,$t2\n");
            else if (h->op == MINUS)
                printf("  sub $t3,$t1,$t2\n");
            else if (h->op == STAR){
                // printf("  mul $t3,$t1,$t2\n");
                printf("  mult $t1,$t2\n");
                printf("  mflo $t3\n");
            }
            else if (h->op == DIV) {
                printf("  div $t1,$t2\n"); //商存在hi
                printf("  mflo $t3\n");     //将hi中数据存到$t3
            }
            //存运算结果
            printf("  sw $t3,%d($sp)\n", h->result.offset);
            break;
        case COMADD:
        case COMSUB:
        case COMSTAR:
        case COMDIV:
            printf("  lw $t1,%d($sp)\n", h->result.offset);
            if (h->opn1.kind == INT)
                //加载立即数（操作数）到$t2
                printf("  li $t2,%d\n", h->opn1.const_int);
            else 
                //加载操作数到$t2
                printf("  lw $t2,%d($sp)\n", h->opn1.offset);
            //运算符
            if (h->op == COMADD)
                printf("  add $t3,$t1,$t2\n");
            else if (h->op == COMSUB)
                printf("  sub $t3,$t1,$t2\n");
            else if (h->op == COMSTAR){
                // printf("  mul $t3,$t1,$t2\n");
                printf("  mult $t1,$t2\n");
                printf("  mflo $t3\n");
            }
            else if (h->op == COMDIV) {
                printf("  div $t1,$t2\n"); //商存在hi
                printf("  mflo $t3\n");     //将hi中数据存到$t3
            }
            //从$t3存运算结果
            printf("  sw $t3,%d($sp)\n", h->result.offset);
            break;
        case FUNCTION:
            //函数名
            printf("\n%s:\n", h->result.id);    //函数标号
            //特殊处理main函数
            if (!strcmp(h->result.id, "main")) {
                //将栈顶指针移到main函数出
                printf("  addi $sp,$sp,-%d\n",
                       symbolTable.symbols[h->result.offset].offset);
            }
            break;
        case SELFADD_R:
        case SELFSUB_R:
            //加载操作数
            printf("  lw $t1,%d($sp)\n", h->opn1.offset);
            printf("  sw $t1,%d($sp)\n", h->result.offset);
            //+1 -1
            if(h->op == SELFADD_R)
                printf("  addi $t1,$t1,1\n");
            else
                printf("  addi $t1,$t1,-1\n");
            //存运算结果
            printf("  sw $t1,%d($sp)\n", h->opn1.offset);
            break;
        case UMINUS:
            printf(" li $t1,0\n");
            //加载操作数
            printf("  lw $t2,%d($sp)\n", h->opn1.offset);
            printf("  sub $t3,$t1,$t2\n");     //0-操作数1
            printf("  sw $t3,%d($sp)\n", h->result.offset);
            break;
        case PARAM: //直接跳到下条TAC
            break;
        case LABEL:
            //输出标号
            printf("%s:\n", h->result.id);
            break;
        case GOTO:
            //无条件跳转
            printf("  j %s\n", h->result.id);
            break;
        case JLE:   // <=
        case JLT:   // <
        case JGE:   //  >=
        case JGT:   // >
        case EQ:    // ==
        case NEQ:   // !=
            //操作数1
            if (h->opn1.kind == ID)
                printf("  lw $t1,%d($sp)\n", h->opn1.offset);
            else
                printf("  li $t1,%d\n", h->opn2.const_int);
            //操作数2
            if (h->opn2.kind == ID)
                printf("  lw $t2,%d($sp)\n", h->opn2.offset);
            else
                printf("  li $t2,%d\n", h->opn2.const_int);
            //运算符
            if (h->op == JLE)
                printf("  ble $t1,$t2,%s\n", h->result.id);
            else if (h->op == JLT)
                printf("  blt $t1,$t2,%s\n", h->result.id);
            else if (h->op == JGE)
                printf("  bge $t1,$t2,%s\n", h->result.id);
            else if (h->op == JGT)
                printf("  bgt $t1,$t2,%s\n", h->result.id);
            else if (h->op == EQ)
                printf("  beq $t1,$t2,%s\n", h->result.id);
            else
                printf("  bne $t1,$t2,%s\n", h->result.id);
            break;
        case EXP_LIST: //直接跳到下条TAC,回头反查
            break;
        case CALL:
            //调用read函数
            if (!strcmp(h->opn1.id, "read")) {
                printf("  addi $sp,$sp,-4\n");
                printf("  sw $ra,0($sp)\n");
                printf("  jal read\n");     //子程序调用，$sp存档$ra
                printf("  lw $ra,0($sp)\n");
                printf("  addi $sp,$sp,4\n");
                printf("  sw $v0,%d($sp)\n", h->result.offset);
                break;
            }
            //调用write函数
            if (!strcmp(h->opn1.id, "write")) {
                printf("  lw $a0, %d($sp)\n", h->prior->result.offset);
                printf("  addi $sp, $sp, -4\n");
                printf("  sw $ra,0($sp)\n");
                printf("  jal write\n");
                printf("  lw $ra,0($sp)\n");
                printf("  addi $sp, $sp, 4\n");
                break;
            }
            //定位到第一个实参的结点
            for (p = h, i = 0;
                 i < symbolTable.symbols[h->opn1.offset].paramnum;
                 i++)
                p = p->prior;
            //开活动记录空间
            //保存当前函数的sp到$t0中，用于取实参表达式的值
            printf("  move $t0,$sp\n");
            //移动栈顶指针
            printf("  addi $sp,$sp,-%d\n", symbolTable.symbols[h->opn1.offset].offset);
            printf("  sw $ra,0($sp)\n");    //保留返回地址
            i = h->opn1.offset + 1;      //第一个形参变量在符号表的位置序号
            //遍历参数
            while (symbolTable.symbols[i].flag == 'P') {
                //加载参数
                printf("  lw $t1,%d($t0)\n", p->result.offset);
                printf("  move $t3,$t1\n");
                printf("  sw $t3,%d($sp)\n", symbolTable.symbols[i].offset); //送到被调用函数的形参单元
                p = p->next;
                i++;
            }
            printf("  jal %s\n", h->opn1.id);   //跳转至函数                             //恢复返回地址
            printf("  lw $ra,0($sp)\n");    //提出原栈顶指针                                         //恢复返回地址
            printf("  addi $sp,$sp,%d\n",
                   symbolTable.symbols[h->opn1.offset].offset); //释放活动记录空间
            if(h->result.kind != VOID)
                printf("  sw $v0,%d($sp)\n", h->result.offset); //取返回值
            break;
        case RETURN:
            //存返回值到寄存器
            printf("  lw $v0,%d($sp)\n", h->result.offset);
        case RETURN_E:
            printf("  jr $ra\n");   //跳转出函数到原栈顶
            break;
        }
        h = h->next;
    } while (h != head);
}