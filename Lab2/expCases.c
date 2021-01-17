/*  处理 semanticAnalysis 中的 exp */
#include "semantic.h"

extern int LEV;       //层号
extern int func_size; //1个函数的活动记录大小

//布尔表达式 语义分析
void boolExp(struct Node *T) { 
    struct OperandStruct opn1, opn2, result;
    int op;
    int rtn;
    if (T) {
        switch (T->kind) {
        case INT:
            T->width = 0;
            break;
        case FLOAT:
            T->width = 0;
            break;
        case ID: //查符号表，获得符号表中的位置，类型送type
            rtn = searchSymbolTable(T->type_id);
            if (rtn == -1) 
                semanticError(T->pos, T->type_id, "变量未定义");
            if (symbolTable.symbols[rtn].flag == 'F')
                semanticError(T->pos, T->type_id, "是函数名，类型不匹配");
            T->width = 0;
            break;
        case RELOP: //处理关系运算表达式,2个操作数都按基本表达式处理
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
            Exp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            Exp(T->ptr[1]);
            if (T->width < T->ptr[1]->width)
                T->width = T->ptr[1]->width;
           break;
        case AND:
        case OR:
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
            boolExp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            boolExp(T->ptr[1]);
            if (T->width < T->ptr[1]->width)
                T->width = T->ptr[1]->width;
            break;
        case NOT:
            boolExp(T->ptr[0]);
            break;
        default:
            break;
        }
    }
}

//变量 语义分析
void id_exp(struct Node *T) {
    int ret, num, width;
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
    }
}

//VOID 类型 语义分析
void void_exp(struct Node *T) {
    T->type = VOID;
    T->width = 0;
}

//int类型常量 语义分析
void int_exp(struct Node *T) {
    T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset); //为整常量生成一个临时变量
    T->type = INT;
    T->width = 4;   ///新的临时int宽度
}

//float类型常量 语义分析
void float_exp(struct Node *T) {
    T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset); //为浮点常量生成一个临时变量
    T->type = FLOAT;
    T->width = 8;
}

//char类型常量 语义分析
void char_exp(struct Node *T) {
    // int ret, num, width;
    // struct OperandStruct opn1, opn2, result;
    T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset); //为整常量生成一个临时变量
    T->type = CHAR;
    T->width = 1;
}

//赋值语句 语义分析
void assignop_exp(struct Node *T) {
    if (T->ptr[0]->kind != ID) {
        semanticError(T->pos, "", "赋值语句需要左值");
    }
    else {
        Exp(T->ptr[0]); //处理左值
        T->ptr[1]->offset = T->offset;  //左值已定义不需要空间
        Exp(T->ptr[1]);
        T->type = T->ptr[0]->type;
        if (T->ptr[0]->type != T->ptr[1]->type) {
            semanticError(T->pos, "", "赋值语句类型不匹配");
        }
        T->width = T->ptr[1]->width;
    }
}

//比较运算符表达式(算术方法) 语义分析
void relop_exp(struct Node *T) {
    T->type = INT;
    T->ptr[0]->offset = T->offset;
    Exp(T->ptr[0]);
    T->ptr[1]->offset = T->offset + T->ptr[0]->width;
    Exp(T->ptr[1]);
    T->width = T->ptr[0]->width + T->ptr[1]->width + 4;
    T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset);
}

//实参列表 语义分析
void args_exp(struct Node *T) {
    //第一个参数
    T->ptr[0]->offset = T->offset;
    Exp(T->ptr[0]);
    T->width = T->ptr[0]->width;
    //其他参数
    if (T->ptr[1]) {
        T->ptr[1]->offset = T->offset + T->ptr[0]->width;
        Exp(T->ptr[1]);
        T->width += T->ptr[1]->width;
    }
}

// 算数运算：加减乘除
void op_exp(struct Node *T) {
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
    T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset + T->ptr[0]->width + T->ptr[1]->width);
    T->width = T->ptr[0]->width + T->ptr[1]->width + (T->type == INT ? 4 : 8);
}

// && 语义分析
void and_exp(struct Node *T) {
    T->type = INT;
    T->ptr[0]->offset = T->offset;
    Exp(T->ptr[0]);
    T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset);

    T->ptr[1]->offset = T->offset + T->ptr[0]->width;
    Exp(T->ptr[1]);

    T->width = T->ptr[0]->width + T->ptr[1]->width + 4;
}

// || 语义分析
void or_exp(struct Node *T) {
    T->type = INT;
    T->ptr[0]->offset = T->offset;
    Exp(T->ptr[0]);
    T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset);

    T->ptr[1]->offset = T->offset + T->ptr[0]->width;
    Exp(T->ptr[1]);
    T->width = T->ptr[0]->width + T->ptr[1]->width + 4;
}

//右自增/右自减 语义分析
void self_exp(struct Node *T){
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
    }
}

//函数调用 语义分析
void func_call_exp(struct Node *T) {
    int ret, width;
    struct Node *T0;
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
    }
    //无形参
    else {
        T->width = width;
    }
    matchParam(ret, T->ptr[0]); //处理所有参数的匹配
    //处理参数列表的中间代码
    T0 = T->ptr[0];
    while (T0) {
        T0 = T0->ptr[1];
    }
    if (width > 0)
        T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset + T->width - width);
}

//取非 语义分析
void not_exp(struct Node *T) {
    T->type = INT;
    T->place = fillTemp(newTemp(), LEV, INT, 'T', T->offset);
    T->ptr[0]->offset = T->offset + 4;
    Exp(T->ptr[0]);
    T->width = T->ptr[0]->width + 4;
}

//取负 语义分析
void uminus_exp(struct Node *T) {
    int width;
    T->type = T->ptr[0]->type;
    if (T->type == CHAR) {
        semanticError(T->pos, "", "不支持CHAR类型算数计算");
        return;
    }
    else if(T->type == INT) width = 4;
    else if(T->type == FLOAT) width = 8;
    else width = 1;
    T->place = fillTemp(newTemp(), LEV, T->type, 'T', T->offset);
    T->ptr[0]->offset = T->offset + width;
    Exp(T->ptr[0]);
    T->width = T->ptr[0]->width + width;
}
