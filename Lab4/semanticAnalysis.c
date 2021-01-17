#include "semantic.h"

int LEV = 0;       //层号
int func_size; //1个函数的活动记录大小
int is_loop = 0;    //循环标记
int need_return = 0;    //函数返回值标记
char continue_label[15];    //循环标号
char break_label[15];   //break标号

//字符串拼接
char *strcpycat(char *s1, char *s2) {
    static char result[10];
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

//新建别名v%d
char *newAlias() {
    static int no = 1;
    char s[10];
    snprintf(s, 10, "%d", no++);
    return strcpycat("v", s);
}

//新建临时变量
char *newTemp() {
    static int no = 1;
    char s[10];
    snprintf(s, 10, "%d", no++);
    return strcpycat("temp", s);
}

//新建标号名
char *newLabel() {
    static int no = 1;
    char s[10];
    snprintf(s, 10, "%d", no++);
    return strcpycat("label", s);
}

//生成一条标号语句，返回头指针
struct CodeNode *genLabel(char *label) {
    struct CodeNode *h 
        = (struct CodeNode *)malloc(sizeof(struct CodeNode));
    h->op = LABEL;
    strcpy(h->result.id, label);
    h->next = h->prior = h;
    return h;
}

//生成GOTO语句，返回头指针
struct CodeNode *genGoto(char *label) {
    struct CodeNode *h 
        = (struct CodeNode *)malloc(sizeof(struct CodeNode));
    h->op = GOTO;
    strcpy(h->result.id, label);
    h->next = h->prior = h;
    return h;
}

//生成一条TAC代码的结点组成的双向循环链表，返回头指针
struct CodeNode *genIR(int op, struct Operand opn1, struct Operand opn2,
                       struct Operand result) {
    struct CodeNode *h 
        = (struct CodeNode *)malloc(sizeof(struct CodeNode));
    h->op = op;
    h->opn1 = opn1;
    h->opn2 = opn2;
    h->result = result;
    h->next = h->prior = h;
    return h;
}

//合并多个中间代码的双向循环链表，首尾相连
struct CodeNode *merge(int num, ...) {
    struct CodeNode *h1, *h2, *p, *t1, *t2;
    va_list ap;
    va_start(ap, num);
    h1 = va_arg(ap, struct CodeNode *);
    while (--num > 0) {
        h2 = va_arg(ap, struct CodeNode *);
        if (h1 == NULL)
            h1 = h2;
        else if (h2) {
            t1 = h1->prior;
            t2 = h2->prior;
            t1->next = h2;
            t2->next = h1;
            h1->prior = t2;
            h2->prior = t1;
        }
    }
    va_end(ap);
    return h1;
}

//语义分析错误记录
void semanticError(int line, char *msg1, char *msg2) {
    // 这里可以只收集错误信息，最后在一次显示
    printf("在%d行,%s %s\n", line, msg1, msg2);
}

//输出符号表
void printSymbol() {
    int i = 0;
    char *symbolsType;
    printf("  %6s  %6s   %6s   %6s  %4s  %6s\n", "变量名", "别名", "层号", "类型", "标记", "偏移量");
    for (i = 0; i < symbolTable.index; i++) {
        if (symbolTable.symbols[i].type == INT) {
            symbolsType = "int";
        }
        else if (symbolTable.symbols[i].type == FLOAT) {
            symbolsType = "float";
        }
        else if (symbolTable.symbols[i].type == CHAR) {
            symbolsType = "char";
        }
        else if(symbolTable.symbols[i].type == VOID) {
            symbolsType = "void";
        }
        printf("%6s %6s %6d  %6s %4c %6d\n", symbolTable.symbols[i].name,
               symbolTable.symbols[i].alias, symbolTable.symbols[i].level,
               symbolsType,
               symbolTable.symbols[i].flag, symbolTable.symbols[i].offset);
    }
}

//输出中间代码
void printIR(struct CodeNode *head) {
    char opnstr1[32], opnstr2[32], resultstr[32];
    struct CodeNode *h = head;
    do {
        //操作数1
        if (h->opn1.kind == INT)
            sprintf(opnstr1, "#%d", h->opn1.const_int);
        else if (h->opn1.kind == FLOAT)
            sprintf(opnstr1, "#%f", h->opn1.const_float);
        else if (h->opn1.kind == CHAR)
            sprintf(opnstr1, "#%c", h->opn1.const_char);
        else if (h->opn1.kind == ID)
            sprintf(opnstr1, "%s", h->opn1.id);
        if (h->opn2.kind == INT)
            sprintf(opnstr2, "#%d", h->opn2.const_int);
        else if (h->opn2.kind == FLOAT)
            sprintf(opnstr2, "#%f", h->opn2.const_float);
        else if (h->opn2.kind == CHAR)
            sprintf(opnstr2, "#%c", h->opn2.const_char);
        else if (h->opn2.kind == ID)
            sprintf(opnstr2, "%s", h->opn2.id);
        sprintf(resultstr, "%s", h->result.id);
        switch (h->op) {
        case ASSIGNOP:
            printf("  %s := %s\n", resultstr, opnstr1);
            break;
        case COMADD:
        case COMSUB:
        case COMSTAR:
        case COMDIV:
            printf("  %s := %s %c %s\n", resultstr, resultstr,
                   h->op == COMADD ? '+' : h->op == COMSUB ? '-' : h->op == COMSTAR ? '*' : '/',
                   opnstr1);
            break;
        case PLUS:
        case MINUS:
        case STAR:
        case DIV:
            printf("  %s := %s %c %s\n", resultstr, opnstr1,
                   h->op == PLUS ? '+' : h->op == MINUS ? '-' : h->op == STAR ? '*' : '/', opnstr2);
            break;
        case SELFADD_R:
        case SELFSUB_R:
                printf("  %s := %s\n", resultstr, opnstr1);
                printf("  %s := %s %c 1\n", opnstr1, resultstr, h->op == SELFADD_R ? '+' : '-');
            break;
        case UMINUS:
            printf("  %s := #0 - %s\n", resultstr, opnstr1);
            break;
        case FUNCTION:
            printf("\nFUNCTION %s :\n", h->result.id);
            break;
        case PARAM:
            printf("  PARAM %s\n", h->result.id);
            break;
        case LABEL:
            printf("LABEL %s :\n", h->result.id);
            break;
        case GOTO:
            printf("  GOTO %s\n", h->result.id);
            break;
        case JLE:
            printf("  IF %s <= %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JLT:
            printf("  IF %s < %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JGE:
            printf("  IF %s >= %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JGT:
            printf("  IF %s > %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case EQ:
            printf("  IF %s == %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case NEQ:
            printf("  IF %s != %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case EXP_LIST:
            printf("  ARG %s\n", h->result.id);
            break;
        case CALL:
            if (!strcmp(opnstr1, "write") || h->result.kind == VOID)
                printf("  CALL  %s\n", opnstr1);
            else
                printf("  %s := CALL %s\n", resultstr, opnstr1);
            break;
        case RETURN:
        case RETURN_E:
            if (h->result.kind != VOID)
                printf("  RETURN %s\n", resultstr);
            else
                printf("  RETURN\n");
            break;
        }
        h = h->next;
    } while (h != head);
}

//查找符号表 返回在符号表中的位置序号
int searchSymbolTable(char *name) {
    int i;
    //逆序查找
    for (i = symbolTable.index - 1; i >= 0; i--)
        if (!strcmp(symbolTable.symbols[i].name, name))
            return i;
    return -1;
}

// 首先根据name查符号表，不能重复定义 重复定义返回-1
int fillSymbolTable(char *name, char *alias, int level, int type, 
                    char flag, int offset) {

    int i;
    /*符号查重，考虑外部变量声明前有函数定义，
    其形参名还在符号表中，这时的外部变量与前函数的形参重名是允许的*/
    for (i = symbolTable.index - 1;
        //层次相同 或 全局变量
         symbolTable.symbols[i].level == level || (level == 0 && i >= 0);
         i--) {
        //外部变量和形参不必比较重名
        if (level == 0 && symbolTable.symbols[i].level == 1) continue; 
        //重名返回-1
        if (!strcmp(symbolTable.symbols[i].name, name)) return -1;
    }
    //填写符号表内容
    strcpy(symbolTable.symbols[symbolTable.index].name, name);  //名称
    strcpy(symbolTable.symbols[symbolTable.index].alias, alias);    //别名
    symbolTable.symbols[symbolTable.index].level = level;   //层次
    symbolTable.symbols[symbolTable.index].type = type;     //类型
    symbolTable.symbols[symbolTable.index].flag = flag;     //标记
    symbolTable.symbols[symbolTable.index].offset = offset; //偏移
    return symbolTable.index++;
    //返回的是符号在符号表中的位置序号，中间代码生成时可用序号取到符号别名
}

//填写临时变量到符号表，返回临时变量在符号表中的位置
int fillTemp(char *name, int level, int type, char flag, int offset) {
    strcpy(symbolTable.symbols[symbolTable.index].name, "");
    strcpy(symbolTable.symbols[symbolTable.index].alias, name);
    symbolTable.symbols[symbolTable.index].level = level;
    symbolTable.symbols[symbolTable.index].type = type;
    symbolTable.symbols[symbolTable.index].flag = flag;
    symbolTable.symbols[symbolTable.index].offset = offset;
    return symbolTable.index++; //返回的是临时变量在符号表中的位置序号
}

//匹配函数参数
int matchParam(int i, struct Node *T) { 
    int j, num = symbolTable.symbols[i].paramnum;
    int type1, type2;
    if (num == 0 && T == NULL) return 1;
    for (j = 1; j < num; j++) {
        if (!T) {
            semanticError(T->pos, "", "函数调用参数过少");
            return 0;
        }
        type1 = symbolTable.symbols[i + j].type; //形参类型
        type2 = T->ptr[0]->type;    //实参类型
        if (type1 != type2) {
            semanticError(T->pos, "", "函数参数类型不匹配");
            return 0;
        }
        T = T->ptr[1];
    }
    if (T->ptr[1]) {    //num个参数已经匹配完，还有实参表达式
        semanticError(T->pos, "", "函数调用参数过多");
        return 0;
    }
    return 1;
}

//基本表达式 语义分析
void Exp(struct Node *T) {
    if (T) {
        switch (T->kind) {
        case ID:        //标识符类型
            //查符号表，获得符号表中的位置，类型送type
            id_exp(T);
            break;
        case INT:       //int
            int_exp(T);
            break;
        case FLOAT:     //float
            float_exp(T);
            break;
        case CHAR:      //char
            char_exp(T);
            break;
        case VOID:
            void_exp(T);
            break;
        case COMADD:    // +=
        case COMSUB:    // -=
        case COMSTAR:   // *=
        case COMDIV:    // /=
            com_exp(T);
            break;
        case ASSIGNOP:  //赋值语句
            assignop_exp(T);
            break;
        case RELOP:     //比较运算符
            relop_exp(T);
            break;
        case PLUS:      //算术运算
        case MINUS:
        case STAR:
        case DIV:
            op_exp(T);
            break;
        case SELFADD_R: //后自增
        case SELFSUB_R: //后自减
            self_exp(T);
            break;
        //按算术表达式方式计算布尔值
        case AND:       // &&
            and_exp(T);
            break;
        case OR:        // ||
            or_exp(T);
            break;
        case NOT:       // !
            not_exp(T);
            break;
        case UMINUS:    // -
            uminus_exp(T);
            break;
        case FUNC_CALL: //函数调用
            //根据T->type_id查出函数的定义，如果语言中增加了实验教材的read，write需要单独处理一下
            func_call_exp(T);
            break;
        case EXP_LIST:  //表达式列表（实参列表）
            //此处仅处理各实参表达式的求值的代码序列，不生成Exp_List的实参系列
            args_exp(T);
            break;
        }
    }
}

//对抽象语法树的先根遍历,按display的控制结构修改完成符号表管理和语义检查和TAC生成（语句部分）
void semanticAnalysis(struct Node *T) { 
    if (T) {
        switch (T->kind) {
        case EXT_DEF_LIST:      //外部定义列表
            ext_def_list(T);
            break;
        case EXT_VAR_DEF:       //外部变量定义语句
            //处理外部说明,将第一个孩子(TYPE结点)中的类型送到第二个孩子的类型域
            ext_var_def(T);
            break;
        case FUNC_DEF:          //函数定义语句
            func_def(T);
            break;
        case FUNC_DEC:          //函数定义
            func_dec(T);    //根据返回类型，函数名填写符号表
            break;
        case PARAM_LIST:        //处理函数形参列表
            param_list(T);
            break;
        case PARAM_DEC:         //函数形参定义
            param_dec(T);
            break;
        case COMP_STM:
            comp_stm(T);        //复合语句
            break;
        case DEF_LIST:          //局部定义列表
            def_list(T);
            break;
        case VAR_DEF:           //局部变量定义语句
            //处理一个局部变量定义,将第一个孩子(TYPE结点)中的类型送到第二个孩子的类型域
            //类似于上面的外部变量EXT_VAR_DEF，换了一种处理方法
            var_def(T);
            break;
        case STM_LIST:          //语句列表
            stm_list(T);        
            break;
        case IF_THEN:           //if语句
            if_then(T);
            break;
        case IF_THEN_ELSE:      //if-else语句
            if_then_else(T);
            break;
        case WHILE:             //while语句
            while_dec(T);
            break;
        case EXP_STMT:          //表达式语句
            exp_stmt(T);
            break;
        case RETURN_E:          //return语句
        case RETURN:
            return_dec(T);
            break;
        case BREAK:             //break语句
            break_exp(T);
            break;
        case CONTINUE:          //continue语句
            continue_exp(T);
            break;
        case ID:
        case INT:
        case FLOAT:
        case CHAR:
        case VOID:
        case ASSIGNOP:
        case AND:
        case OR:
        case RELOP:
        case PLUS:
        case SELFADD_R:
        case COMADD:
        case MINUS:
        case SELFSUB_R:
        case COMSUB:
        case STAR:
        case COMSTAR:
        case DIV:
        case COMDIV:
        case NOT:
        case UMINUS:
        case FUNC_CALL:
            Exp(T);     //处理基本表达式
            break;
        }
    }
}

//语义分析初始化
void semanticAnalysisInit(struct Node *T) {
    symbolTable.index = 0;
    fillSymbolTable("read", "", 0, INT, 'F', 4);
    symbolTable.symbols[0].paramnum = 0;    //read的形参个数
    fillSymbolTable("x", "", 1, INT, 'P', 12);
    fillSymbolTable("write", "", 0, INT, 'F', 4);
    symbolTable.symbols[2].paramnum = 1;
    symbolScopeStack.scopeArray[0] = 0;     //外部变量在符号表中的起始序号为0
    symbolScopeStack.top = 1;
    T->offset = 0; // 外部变量在数据区的偏移量
    // printf("\n符号表：\n");
    semanticAnalysis(T);
    // printSymbol();  //输出符号表
    // printf("\n中间代码：\n");
    // printIR(T->code);   //生成中间代码
    // printf("\n目标代码：\n");
    objectCode(T->code);
}