/*  处理 semanticAnalysis 中的 exp */
#include "semantic.h"

extern int LEV;       //层号
extern int func_size; //1个函数的活动记录大小

//布尔表达式 语义分析
void boolExp(struct Node *T) { 
    struct Operand opn1, opn2, result;
    int op;
    int ret;
    if (T) {
        switch (T->kind) {
        case INT:
            T->width = 0;
            if (T->type_int != 0)
                T->code = genGoto(T->eTrue);
            else
                T->code = genGoto(T->eFalse);
            break;
        case FLOAT:
            T->width = 0;
            if (T->type_float != 0.0)
                T->code = genGoto(T->eTrue);
            else
                T->code = genGoto(T->eFalse);
            break;
        case CHAR:
            T->width = 0;
            if(T->type != '\0')
                T->code = genGoto(T->eTrue);
            else
                T->code = genGoto(T->eFalse);
            break;
        case ID: //查符号表，获得符号表中的位置，类型送type
            ret = searchSymbolTable(T->type_id);
            if (ret == -1) 
                semanticError(T->pos, T->type_id, "变量未定义");
            if (symbolTable.symbols[ret].flag == 'F')
                semanticError(T->pos, T->type_id, "是函数名，类型不匹配");
            else {
                //操作数1是变量
                opn1.kind = ID;
                strcpy(opn1.id, symbolTable.symbols[ret].alias);
                opn1.offset = symbolTable.symbols[ret].offset;
                //操作数2是常数0
                opn2.kind = INT;
                opn2.const_int = 0;
                //运算结果是真出口标号
                result.kind = ID;
                strcpy(result.id, T->eTrue);
                //生成判断中间代码
                T->code = genIR(NEQ, opn1, opn2, result);
                //合并判断、跳转到假出口的中间代码
                T->code = merge(2, T->code, genGoto(T->eFalse));
            }
            T->width = 0;
            break;
        case RELOP: //处理关系运算表达式,2个操作数都按基本表达式处理
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
            Exp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            Exp(T->ptr[1]);
            if (T->width < T->ptr[1]->width)
                T->width = T->ptr[1]->width;
            //两个操作数为比较符左右符号
            opn1.kind = ID;
            strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
            opn1.offset = symbolTable.symbols[T->ptr[0]->place].offset;
            opn2.kind = ID;
            strcpy(opn2.id, symbolTable.symbols[T->ptr[1]->place].alias);
            opn2.offset = symbolTable.symbols[T->ptr[1]->place].offset;
            //结果为真出口标号
            result.kind = ID;
            strcpy(result.id, T->eTrue);
            if (strcmp(T->type_id, "<") == 0)
                op = JLT;
            else if (strcmp(T->type_id, "<=") == 0)
                op = JLE;
            else if (strcmp(T->type_id, ">") == 0)
                op = JGT;
            else if (strcmp(T->type_id, ">=") == 0)
                op = JGE;
            else if (strcmp(T->type_id, "==") == 0)
                op = EQ;
            else if (strcmp(T->type_id, "!=") == 0)
                op = NEQ;
            //生成判断的中间代码
            T->code = genIR(op, opn1, opn2, result);
            //合并两个操作数、判断、跳转到假出口的中间代码
            T->code = merge(4, T->ptr[0]->code, T->ptr[1]->code, T->code,
                            genGoto(T->eFalse));
            break;
        case AND:
        case OR:
            if (T->kind == AND) {
                //生成真出口标号
                strcpy(T->ptr[0]->eTrue, newLabel());
                //AND的假出口也为第一个表达式的假出口
                strcpy(T->ptr[0]->eFalse, T->eFalse);
            }
            else {
                //OR的真出口也为第一个表达式的真出口
                strcpy(T->ptr[0]->eTrue, T->eTrue);
                //生成假出口标号
                strcpy(T->ptr[0]->eFalse, newLabel());
            }
            //第2个表达式的真假出口与语句的真假出口一致
            strcpy(T->ptr[1]->eTrue, T->eTrue);
            strcpy(T->ptr[1]->eFalse, T->eFalse);
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
            //对1、2表达式bool判断
            boolExp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            boolExp(T->ptr[1]);
            if (T->width < T->ptr[1]->width)
                T->width = T->ptr[1]->width;

            if (T->kind == AND)
                //AND 合并 第1个表达式、1表达式真出口标号、2表达式的中间代码
                T->code = merge(3, T->ptr[0]->code,
                                genLabel(T->ptr[0]->eTrue), T->ptr[1]->code);
            else
                //OR 合并 第1表达式、1表达式假出口标号、2表达式的中间代码
                T->code = merge(3, T->ptr[0]->code,
                                genLabel(T->ptr[0]->eFalse), T->ptr[1]->code);
            break;
        case NOT:
            //真假出口调换
            strcpy(T->ptr[0]->eTrue, T->eFalse);
            strcpy(T->ptr[0]->eFalse, T->eTrue);
            boolExp(T->ptr[0]);
            break;
        default:
            break;
        }
    }
}

//变量 语义分析
void id_exp(struct Node *T) {
    int ret;
    ret = searchSymbolTable(T->type_id);
    if (ret == -1) {
        semanticError(T->pos, T->type_id, "变量未定义");
    }
    else if (symbolTable.symbols[ret].flag == 'F')
        semanticError(T->pos, T->type_id, "变量是函数名，类型不匹配");
    else {
        T->place = ret; //结点保存变量在符号表中的位置
        //确定标识符的类型和偏移
        T->type = symbolTable.symbols[ret].type;
        T->offset = symbolTable.symbols[ret].offset;
        T->width = 0;   //未再使用新单元
        T->code = NULL; //标识符不需要生成TAC
    }
}

//VOID 类型 语义分析
void void_exp(struct Node *T) {
    T->type = VOID;
    T->width = 0;
}

//int类型常量 语义分析
void int_exp(struct Node *T) {
    struct Operand opn1, opn2, result;
    T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset); //为整常量生成一个临时变量
    T->type = INT;
    T->width = 4;   ///新的临时int宽度
    opn1.kind = INT;    //操作数1为int
    opn1.const_int = T->type_int;   //int值
    result.kind = ID;   //结果类型为标识符
    strcpy(result.id, symbolTable.symbols[T->place].alias); //结果名为临时变量别名
    result.offset = symbolTable.symbols[T->place].offset;   //结果偏移
    T->code = genIR(ASSIGNOP, opn1, opn2, result);  //生成中间代码
}

//float类型常量 语义分析
void float_exp(struct Node *T) {
    struct Operand opn1, opn2, result;
    T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset); //为浮点常量生成一个临时变量
    T->type = FLOAT;
    T->width = 8;
    opn1.kind = FLOAT;
    opn1.const_float = T->type_float;
    result.kind = ID;
    strcpy(result.id, symbolTable.symbols[T->place].alias);
    result.offset = symbolTable.symbols[T->place].offset;
    T->code = genIR(ASSIGNOP, opn1, opn2, result);
}

//char类型常量 语义分析
void char_exp(struct Node *T) {
    struct Operand opn1, opn2, result;
    T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset); //为整常量生成一个临时变量
    T->type = CHAR;
    T->width = 1;
    opn1.kind = CHAR;
    opn1.const_char = T->type_char;
    result.kind = ID;
    strcpy(result.id, symbolTable.symbols[T->place].alias);
    result.offset = symbolTable.symbols[T->place].offset;
    T->code = genIR(ASSIGNOP, opn1, opn2, result);
}

//赋值语句 语义分析
void assignop_exp(struct Node *T) {
    struct Operand opn1, opn2, result;
    if (T->ptr[0]->kind != ID) {
        semanticError(T->pos, "", "赋值语句需要左值");
    }
    else {
        Exp(T->ptr[0]); //处理左值
        T->ptr[1]->offset = T->offset;  //左值已定义不需要空间
        Exp(T->ptr[1]);
        T->type =  T->ptr[0]->type;
        if (T->ptr[0]->type != T->ptr[1]->type) {
            semanticError(T->pos, "", "赋值语句类型不匹配");
        }
        T->width = T->ptr[1]->width;
        //合并赋值语句左值和右值的中间代码
        T->code = merge(2, T->ptr[0]->code, T->ptr[1]->code);
        //操作数1是右值（一定是个变量或临时变量）
        opn1.kind = ID;
        strcpy(opn1.id, symbolTable.symbols[T->ptr[1]->place].alias);
        opn1.offset = symbolTable.symbols[T->ptr[1]->place].offset;
        opn1.type = T->ptr[1]->type;
        //运算结果一定是变量（左值）
        result.kind = ID;
        strcpy(result.id, symbolTable.symbols[T->ptr[0]->place].alias);
        result.offset = symbolTable.symbols[T->ptr[0]->place].offset;
        result.type = T->type;
        //合并左右值、赋值运算中间代码
        T->code = merge(2, T->code, genIR(T->kind, opn1, opn2, result));
    }
}

//+= -= *= /= 语义分析
void com_exp(struct Node *T) {
    struct Operand opn1, opn2, result;
    if (T->ptr[0]->kind != ID) {
        semanticError(T->pos, "", "复合赋值语句需要左值");
    }
    else {
        Exp(T->ptr[0]);                //处理左值
        T->ptr[1]->offset = T->offset; //左值已定义不需要空间
        Exp(T->ptr[1]);
        if (T->ptr[0]->type == CHAR || T->ptr[1]->type == CHAR) {
            semanticError(T->pos, "", "不支持CHAR类型算数计算");
            return;
        }
      
        T->type = T->ptr[0]->type;
        if (T->ptr[0]->type != T->ptr[1]->type) {
            semanticError(T->pos, "", "赋值语句类型不匹配");
        }
        T->width = T->ptr[1]->width;

        //合并语句左值和右值的中间代码
        T->code = merge(2, T->ptr[0]->code, T->ptr[1]->code);
        //操作数1是右值（一定是个变量或临时变量）
        opn1.kind = ID;
        strcpy(opn1.id, symbolTable.symbols[T->ptr[1]->place].alias);
        opn1.offset = symbolTable.symbols[T->ptr[1]->place].offset;
        opn1.type = T->ptr[1]->type;
        //运算结果一定是变量（左值）
        result.kind = ID;
        strcpy(result.id, symbolTable.symbols[T->ptr[0]->place].alias);
        result.offset = symbolTable.symbols[T->ptr[0]->place].offset;
        result.type = T->ptr[0]->type;
        //合并左右值、复合运算中间代码
        T->code = merge(2, T->code, genIR(T->kind, opn1, opn2, result));
    }
}

//比较运算符表达式(算术方法) 语义分析 wchg
void relop_exp(struct Node *T) {
    int op;
    struct Operand opn1, opn2, result, result2;
    char label1[15];
    char label2[15];
    T->type = INT;
    T->ptr[0]->offset = T->offset;
    Exp(T->ptr[0]);
    T->ptr[1]->offset = T->offset + T->ptr[0]->width;
    Exp(T->ptr[1]);
    T->width = T->ptr[0]->width + T->ptr[1]->width + 4;
    //生成表达式结果的临时变量
    T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset);
    //合并比较符左右表达式的中间代码, 并作为左右操作数
    T->code = merge(2, T->ptr[0]->code, T->ptr[1]->code);
    //跳转判断 中间代码
    opn1.kind = ID;
    strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
    opn1.offset = symbolTable.symbols[T->ptr[0]->place].offset;
    opn2.kind = ID;
    strcpy(opn2.id, symbolTable.symbols[T->ptr[1]->place].alias);
    opn2.offset = symbolTable.symbols[T->ptr[1]->place].offset;
    result.kind = ID;
    strcpy(result.id, newLabel());      //运算结果=1标号
    if (strcmp(T->type_id, "<") == 0)
        op = JLT;
    else if (strcmp(T->type_id, "<=") == 0)
        op = JLE;
    else if (strcmp(T->type_id, ">") == 0)
        op = JGT;
    else if (strcmp(T->type_id, ">=") == 0)
        op = JGE;
    else if (strcmp(T->type_id, "==") == 0)
        op = EQ;
    else if (strcmp(T->type_id, "!=") == 0)
        op = NEQ;
    //合并左右表达式，跳转到标号的中间代码
    T->code = merge(2, T->code, genIR(op, opn1, opn2, result));
    //转换成算术值 中间代码
    //操作数1是常数0
    opn1.kind = INT;
    opn1.const_int = 0;
    result2.kind = ID;
    //result2的id为比较运算存结果的临时变量别名
    strcpy(result2.id, symbolTable.symbols[T->place].alias);
    result2.offset = symbolTable.symbols[T->place].offset;
    strcpy(label2, newLabel());     //语句出口标号
    //合并 前面的中间代码、运算结果=0、跳转到语句出口标号、运算结果=1标号的中间代码
    T->code = merge(4, T->code, genIR(ASSIGNOP, opn1, opn2, result2),
                    genGoto(label2), genLabel(result.id));
    opn1.const_int = 1;
    //合并 前面中间代码、运算结果=1、语句出口标号的中间代码
    T->code = merge(3, T->code, genIR(ASSIGNOP, opn1, opn2, result2),
                    genLabel(label2));
}

// 算数运算：加减乘除
void op_exp(struct Node *T) {
    struct Operand opn1, opn2, result;
    T->ptr[0]->offset = T->offset;
    Exp(T->ptr[0]);
    T->ptr[1]->offset = T->offset + T->ptr[0]->width;
    Exp(T->ptr[1]);
    //判断T->ptr[0]，T->ptr[1]类型是否正确，可能根据运算符生成不同形式的代码，给T的type赋值
    //下面的类型属性计算，没有考虑错误处理情况
    if(T->ptr[0]->type == CHAR || T->ptr[1]->type == CHAR) {
        semanticError(T->pos, "", "不支持CHAR类型算数计算");
        return;
    }
    else if (T->ptr[0]->type == FLOAT || T->ptr[1]->type == FLOAT) {
        T->type = FLOAT;
    }
    else {
        T->type = INT;
    }
    //生成临时变量
    T->place = fillTemp(newTemp(), LEV, T->type, 'T',
                        T->offset + T->ptr[0]->width + T->ptr[1]->width);
    T->width = T->ptr[0]->width + T->ptr[1]->width + (T->type == INT ? 4 : 8);

    opn1.kind = ID;
    strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
    opn1.type = T->ptr[0]->type;
    opn1.offset = symbolTable.symbols[T->ptr[0]->place].offset;
    opn2.kind = ID;
    strcpy(opn2.id, symbolTable.symbols[T->ptr[1]->place].alias);
    opn2.type = T->ptr[1]->type;
    opn2.offset = symbolTable.symbols[T->ptr[1]->place].offset;
    result.kind = ID;
    strcpy(result.id, symbolTable.symbols[T->place].alias);
    result.type = T->type;  // + - * /
    result.offset = symbolTable.symbols[T->place].offset;
    //合并左右操作数、算术运算的中间代码
    T->code = merge(3, T->ptr[0]->code, T->ptr[1]->code, genIR(T->kind, opn1, opn2, result));
}

// && 语义分析
void and_exp(struct Node *T) {
    char Label1[15], Label2[15];
    struct Operand opn1, opn2, result;
    T->type = INT;
    T->ptr[0]->offset = T->offset;
    Exp(T->ptr[0]);
    T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset);

    T->code = T->ptr[0]->code;      //第1个表达式中间代码
    strcpy(Label1, newLabel());     //运算结果=0 标号
    strcpy(Label2, newLabel());     //语句出口 标号
    //操作数1为第1个表达式
    opn1.kind = ID;
    strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
    opn1.offset = symbolTable.symbols[T->ptr[0]->place].offset;
    opn1.type = T->ptr[0]->type;
    //操作数2为常数0
    opn2.kind = INT;
    opn2.const_int = 0;
    opn2.type = INT;
    //运算结果=0标号
    result.kind = ID;
    strcpy(result.id, Label1);
    //合并 1表达式、1表达式=0跳转的中间代码
    T->code = merge(2, T->code, genIR(EQ, opn1, opn2, result));

    T->ptr[1]->offset = T->offset + T->ptr[0]->width;
    Exp(T->ptr[1]);
    //合并2表达式的中间代码
    T->code = merge(2, T->code, T->ptr[1]->code);
    //操作数1是2表达式
    opn1.kind = ID;
    strcpy(opn1.id, symbolTable.symbols[T->ptr[1]->place].alias);
    opn1.offset = symbolTable.symbols[T->ptr[1]->place].offset;
    opn1.type = T->ptr[1]->type;
    //合并 2表达式=0跳转的中间代码
    T->code = merge(2, T->code, genIR(EQ, opn1, opn2, result));
    //操作数1改为1
    opn1.kind = INT;
    opn1.const_int = 1;
    opn1.type = INT;
    //运算结果为表达式语句的临时变量别名
    result.kind = ID;
    strcpy(result.id, symbolTable.symbols[T->place].alias);
    result.type = T->type;
    result.offset = symbolTable.symbols[T->place].offset;
    //合并 运算结果=1、跳转到语句出口标号、运算结果=0标号的中间代码
    T->code = merge(4, T->code, genIR(ASSIGNOP, opn1, opn2, result),
                    genGoto(Label2), genLabel(Label1));
    opn1.kind = INT;
    opn1.const_int = 0;
    opn1.type = INT;
    //合并 运算结果=0、语句出口标号的中间代码
    T->code = merge(3, T->code, genIR(ASSIGNOP, opn1, opn2, result),
                    genLabel(Label2));

    T->width = T->ptr[0]->width + T->ptr[1]->width + 4;
}

// || 语义分析
void or_exp(struct Node *T) {
    char Label1[15], Label2[15];
    struct Operand opn1, opn2, result;
    T->type = INT;
    T->ptr[0]->offset = T->offset;
    Exp(T->ptr[0]);
    T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset);
    //1表达式中间代码
    T->code = T->ptr[0]->code;
    strcpy(Label1, newLabel());     //运算结果=1 标号
    strcpy(Label2, newLabel());     //语句出口 标号
    //操作数1是1表达式
    opn1.kind = ID;
    strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
    opn1.offset = symbolTable.symbols[T->ptr[0]->place].offset;
    opn1.type = T->ptr[0]->type;
    //操作数2为常数0
    opn2.kind = INT;
    opn2.const_int = 0;
    opn2.type = INT;
    //运算结果1标号
    result.kind = ID;
    strcpy(result.id, Label1);
    //合并 1表达式！=0跳转的中间代码
    T->code = merge(2, T->code, genIR(NEQ, opn1, opn2, result));

    T->ptr[1]->offset = T->offset + T->ptr[0]->width;
    Exp(T->ptr[1]);
    //表达式2的中间代码
    T->code = merge(2, T->code, T->ptr[1]->code);
    //操作数1是2表达式
    opn1.kind = ID;
    strcpy(opn1.id, symbolTable.symbols[T->ptr[1]->place].alias);
    opn1.offset = symbolTable.symbols[T->ptr[1]->place].offset;
    opn1.type = T->ptr[1]->type;
    //合并 2表达式！=0跳转的中间代码
    T->code = merge(2, T->code, genIR(NEQ, opn1, opn2, result));
    //操作数1改为常数0
    opn1.kind = INT;
    opn1.const_int = 0;
    opn1.type = INT;
    //运算结果为语句临时变量别名
    result.kind = ID;
    strcpy(result.id, symbolTable.symbols[T->place].alias);
    result.type = T->type;
    result.offset = symbolTable.symbols[T->place].offset;
    //合并 运算结果=0、跳转到语句出口、运算结果=1标号
    T->code = merge(4, T->code, genIR(ASSIGNOP, opn1, opn2, result),
                    genGoto(Label2), genLabel(Label1));
    opn1.kind = INT;
    opn1.const_int = 1;
    opn1.type = INT;
    //合并 运算结果=1、语句出口标号
    T->code = merge(3, T->code, genIR(ASSIGNOP, opn1, opn2, result),
                    genLabel(Label2));
    T->width = T->ptr[0]->width + T->ptr[1]->width + 4;
}

//右自增/右自减 语义分析
void self_exp(struct Node *T){
    struct Operand opn1, opn2, result;
    if (T->ptr[0]->kind != ID) {
        semanticError(T->pos, "", "操作数左右值不匹配");
    }
    else {
        Exp(T->ptr[0]);
        T->type = T->ptr[0]->type;
        T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset + T->ptr[0]->width);
        if(T->type == CHAR){
             semanticError(T->pos, "", "不支持CHAR类型算数计算");
             return;
        }
        else if(T->type == INT)  T->width = 4;
        else if(T->type == FLOAT) T->width = 8;
        //操作数1为表达式
        opn1.kind = ID;
        strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
        opn1.type = T->ptr[0]->type;
        opn1.offset = symbolTable.symbols[T->ptr[0]->place].offset;
        //运算结果
        result.kind = ID;
        strcpy(result.id, symbolTable.symbols[T->place].alias);
        result.type = T->type;
        result.offset = symbolTable.symbols[T->place].offset;
        strcpy(opn2.id, T->ptr[0]->type_id);
        //合并 表达式、自增/减语句的中间代码
        T->code = merge(2, T->ptr[0]->code, genIR(T->kind, opn1, opn2, result));
    }
}

//函数调用 语义分析
void func_call_exp(struct Node *T) {
    char return_label[15];
    int ret, width;
    struct Node *T0;
    struct Operand opn1, opn2, result;
    ret = searchSymbolTable(T->type_id);
    if (ret == -1) {
        semanticError(T->pos, T->type_id, "函数未定义");
        return;
    }
    if (symbolTable.symbols[ret].flag != 'F') {
        semanticError(T->pos, T->type_id, "不是一个函数");
        return;
    }
    T->type = symbolTable.symbols[ret].type;
    //存放函数返回值的单数字节数
    if(T->type == INT) width = 4;
    else if(T->type == FLOAT) width = 8;
    else if(T->type == CHAR) width = 1;
    else width = 0; //VOID 类型
    //有形参
    if (T->ptr[0]) {
        T->ptr[0]->offset = T->offset;
        Exp(T->ptr[0]);                      //处理所有实参表达式求值及类型
        T->width = T->ptr[0]->width + width; //累加上计算实参使用临时变量的单元数
        T->code = T->ptr[0]->code;
    }
    //无形参
    else {
        T->width = width;
        T->code = NULL;
    }
    matchParam(ret, T->ptr[0]); //处理所有参数的匹配
    //处理参数列表的中间代码
    T0 = T->ptr[0];
    while (T0) {
        result.kind = ID;
        strcpy(result.id, symbolTable.symbols[T0->ptr[0]->place].alias);
        result.offset = symbolTable.symbols[T0->ptr[0]->place].offset;
        //合并EXP_LIST 形参 的中间代码
        T->code = merge(2, T->code, genIR(EXP_LIST, opn1, opn2, result));
        T0 = T0->ptr[1];
    }
    if (width > 0)
        T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset + T->width - width);
    //操作数1
    opn1.kind = ID;
    strcpy(opn1.id, T->type_id); //保存函数名
    opn1.offset = ret;           //这里offset用以保存函数定义入口,在目标代码生成时，能获取相应信息
    //运算结果
    if(T->type != VOID){
        result.kind = ID;
        strcpy(result.id, symbolTable.symbols[T->place].alias);
        result.offset = symbolTable.symbols[T->place].offset;
    }
    else result.kind = VOID;
    //合并形参代码、调用函数的中间代码
    T->code = merge(2, T->code, genIR(CALL, opn1, opn2, result)); //生成函数调用中间代码
}

//取非 语义分析
void not_exp(struct Node *T) {
    char label1[15], label2[15];
    struct Operand opn1, opn2, result;
    T->type = INT;
    //临时变量
    T->place = fillTemp(newTemp(), LEV, INT, 'T', T->offset);
    T->ptr[0]->offset = T->offset + 4;
    Exp(T->ptr[0]); //表达式
    //表达式的中间代码
    T->code = T->ptr[0]->code;
    strcpy(label1,newLabel());      //运算结果=1 标号
    strcpy(label2, newLabel());     //语句出口 标号
    //操作数1为表达式
    opn1.kind = ID;
    strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
    opn1.offset = symbolTable.symbols[T->ptr[0]->place].offset;
    opn1.type = T->ptr[0]->type;
    //操作数2为常数0
    opn2.kind = INT;
    opn2.const_int = 0;
    opn2.type = INT;
    strcpy(result.id, label1);
    //合并 表达式=0跳转的中间代码
    T->code = merge(2, T->code, genIR(EQ, opn1, opn2, result));
    //运算结果改为语句临时变量别名
    result.kind = ID;
    strcpy(result.id, symbolTable.symbols[T->place].alias);
    result.type = T->type;
    result.offset = symbolTable.symbols[T->place].offset;
    //合并 运算结果=0、跳转到语句出口、运算结果=1标号的中间代码
    T->code = merge(4, T->code, genIR(ASSIGNOP, opn2, opn1, result),
                    genGoto(label2), genLabel(label1));
    opn2.const_int = 1;
    //合并 运算结果=1，语句出口标号的中间代码
    T->code = merge(3, T->code, genIR(ASSIGNOP, opn2, opn1, result),
                    genLabel(label2));
    T->width = T->ptr[0]->width + 4;
}

//取负 语义分析
void uminus_exp(struct Node *T) {
    struct Operand opn1, opn2, result;
    int width;
    T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset);
    T->ptr[0]->offset = T->offset + width;
    Exp(T->ptr[0]);
    T->width = T->ptr[0]->width + width;
    T->type = T->ptr[0]->type;
    if (T->type == CHAR) {
        semanticError(T->pos, "", "不支持CHAR类型算数计算");
        return;
    }
    else if(T->type == INT) width = 4;
    else if(T->type == FLOAT) width = 8;
    else width = 1;

    opn1.kind = ID;
    opn1.offset = symbolTable.symbols[T->ptr[0]->place].offset;
    strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
    result.kind = ID;
    result.offset = symbolTable.symbols[T->place].offset;
    strcpy(result.id, symbolTable.symbols[T->place].alias);
    //合并右值表达式、生成负号运算的中间代码
    T->code = merge(2, T->ptr[0]->code, genIR(UMINUS, opn1, opn2, result));
}

//实参列表 语义分析
void args_exp(struct Node *T) {
    //第一个参数
    T->ptr[0]->offset = T->offset;
    Exp(T->ptr[0]);
    T->width = T->ptr[0]->width;
     T->code = T->ptr[0]->code;
    //其他参数
    if (T->ptr[1]) {
        T->ptr[1]->offset = T->offset + T->ptr[0]->width;
        Exp(T->ptr[1]);
        T->width += T->ptr[1]->width;
        T->code = merge(2, T->code, T->ptr[1]->code);
    }
}
