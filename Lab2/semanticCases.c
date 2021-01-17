/* 处理 emanticAnalysis 中的switch case */
#include "semantic.h"


//外部定义列表 语义分析
void ext_def_list(struct Node *T) {
    if (!T->ptr[0])     //第一个外部定义语句
        return;
    T->ptr[0]->offset = T->offset;   // 语义分析之前先设置偏移地址
    semanticAnalysis(T->ptr[0]); //访问外部定义列表中的第一个
    // 其他外部定义语句，可为空
    if (T->ptr[1]) {
        T->ptr[1]->offset = T->ptr[0]->offset + T->ptr[0]->width;
        semanticAnalysis(T->ptr[1]); //访问该外部定义列表中的其它外部定义
    }
}

//外部变量定义语句 语义分析
void ext_var_def(struct Node *T) {
    //根据第一个子结点（变量类型）设定第一个子结点的类型和宽度
    //并记录到语句结点用于下传到后续子结点（变量结点）
    if (!strcmp(T->ptr[0]->type_id, "int")) {
        T->type = T->ptr[1]->type = INT;
        T->ptr[1]->width = 4;
    }
    else if (!strcmp(T->ptr[0]->type_id, "float")) {
        T->type = T->ptr[1]->type = FLOAT;
        T->ptr[1]->width = 8;
    }
    else if (!strcmp(T->ptr[0]->type_id, "char")) {
        T->type = T->ptr[1]->type = CHAR;
        T->ptr[1]->width = 1;
    }
    T->ptr[1]->offset = T->offset;  //这个外部变量的偏移量向下传递
    ext_var_list(T->ptr[1]);        //处理外部变量说明中的标识符序列
    T->width = (T->ptr[1]->width) * T->ptr[1]->num; //计算这个外部变量说明的宽度
}

//处理变量列表
void ext_var_list(struct Node *T) {
    int ret, num = 1;
    switch (T->kind) {
    case EXT_DEC_LIST:  //变量名列表
        T->ptr[0]->type = T->type;      //将类型属性向下传递变量结点
        T->ptr[0]->offset = T->offset;  //外部变量的偏移量向下传递
        ext_var_list(T->ptr[0]);        //第一个结点是VarDec结点
        T->ptr[1]->type = T->type;      //将类型属性向下传递变量结点
        T->ptr[1]->offset = T->offset + T->width;  //外部变量的偏移量向下传递
        T->ptr[1]->width = T->width;     //外部变量的宽度向下传递
        ext_var_list(T->ptr[1]);        //第二个结点仍为一个变量列表ExtDecList
        T->num = 1 + T->ptr[1]->num;    //计算改变量列表的变量数目（1+第二个列表的变量数）
        break;
    case ID:    //标识符ID
        //最后一个变量名填符号表
        ret = fillSymbolTable(T->type_id, newAlias(), LEV, T->type,
                              'V', T->offset);
        if (ret == -1)
            semanticError(T->pos, T->type_id, "变量重复定义");
        else
            T->place = ret;     //设置变量在符合表中的序号
        T->num = 1;     //变量个数为1
        break;
    default:
        break;
    }
}

//函数定义语句 语义分析
void func_def(struct Node *T) {
    //获取函数返回类型送到含函数名、参数的结点
    if (!strcmp(T->ptr[0]->type_id, "int")) {
        T->ptr[1]->type = INT;
        need_return = 1;
    }
    else if (!strcmp(T->ptr[0]->type_id, "float")) {
        T->ptr[1]->type = FLOAT;
        need_return = 1;
    }
    else if (!strcmp(T->ptr[0]->type_id, "char")) {
        T->ptr[1]->type = CHAR;
        need_return = 1;
    }
    else if (!strcmp(T->ptr[0]->type_id, "void")) {
        T->ptr[1]->type = VOID;
    }
    //填写函数定义信息到符号表
    T->width = 0;                   //函数的宽度设置为0，不会对外部变量的地址分配产生影响
    T->offset = DX;                 //设置局部变量在活动记录中的偏移量初值
    semanticAnalysis(T->ptr[1]);    //处理函数名和参数结点部分，这里不考虑用寄存器传递参数
    T->offset += T->ptr[1]->width;  //用形参单元宽度修改函数局部变量的起始偏移量
    T->ptr[2]->offset = T->offset;
    semanticAnalysis(T->ptr[2]);            //处理函数体结点
    if(need_return){
         semanticError(T->pos, T->type_id, "函数缺少返回值");
    }
    //计算活动记录大小:这里offset属性存放的是活动记录大小，不是偏移
    symbolTable.symbols[T->ptr[1]->place].offset
        = T->offset + T->ptr[2]->width;
}

//函数定义 语义分析
void func_dec(struct Node *T) {
    int ret;
    //函数不在数据区中分配单元，偏移量为0
    ret = fillSymbolTable(T->type_id, newAlias(), LEV, T->type, 'F', 0);
    if (ret == -1) {
        semanticError(T->pos, T->type_id, "函数重复定义");
        return;
    }
    else T->place = ret;
    T->offset = DX;     //设置形式参数在活动记录中的偏移量初值
    if (T->ptr[0]) {    //函数有形参
        T->ptr[0]->offset = T->offset;
        semanticAnalysis(T->ptr[0]);    //处理函数参数列表
        T->width = T->ptr[0]->width;
        symbolTable.symbols[ret].paramnum = T->ptr[0]->num; //形参个数
    }
    else    //函数无形参
        symbolTable.symbols[ret].paramnum = 0, T->width = 0;
}

//函数形参列表 语义分析
void param_list(struct Node *T) {
    T->ptr[0]->offset = T->offset;
    semanticAnalysis(T->ptr[0]);    //分析第一个形参
    if (T->ptr[1]) {    //形参不止一个
        T->ptr[1]->offset = T->offset + T->ptr[0]->width;
        semanticAnalysis(T->ptr[1]);
        T->num = T->ptr[0]->num + T->ptr[1]->num;             //统计参数个数
        T->width = T->ptr[0]->width + T->ptr[1]->width;       //累加参数单元宽度
    }
    else {  //形参只有一个
        T->num = T->ptr[0]->num;
        T->width = T->ptr[0]->width;
    }
}

//函数形参定义 语义分析
void param_dec(struct Node *T) {
    int ret;
    ret = fillSymbolTable(T->ptr[1]->type_id, newAlias(), 1, T->ptr[0]->type,
                          'P', T->offset);
    if (ret == -1)
        semanticError(T->ptr[1]->pos, T->ptr[1]->type_id, "函数形参名重复定义");
    else
        T->ptr[1]->place = ret;
    T->num = 1;                                //参数个数计算的初始值
    //参数宽度
    if(T->ptr[0]->type == INT) T->width = 4;
    else if(T->ptr[0]->type == FLOAT) T->width = 8;
    else T->width = 1;
}

//复合语句 语义分析
void comp_stm(struct Node *T) {
    LEV++;
    //设置层号加1，并且保存该层局部变量在符号表中的起始位置在symbolScopeArray
    symbolScopeStack.scopeArray[symbolScopeStack.top++] = symbolTable.index;
    T->width = 0;
    if (T->ptr[0]) {
        T->ptr[0]->offset = T->offset;
        semanticAnalysis(T->ptr[0]);    //处理该层的局部变量DEF_LIST
        T->width += T->ptr[0]->width;
    }
    if (T->ptr[1]) {
        T->ptr[1]->offset = T->offset + T->width;
        strcpy(T->ptr[1]->sNext, T->sNext); //S.next属性向下传递
        semanticAnalysis(T->ptr[1]);       //处理复合语句的语句序列
        T->width += T->ptr[1]->width;
    }
    printSymbol();  //在退出一个符合语句前显示的符号表
    putchar('\n');
    LEV--;          //出复合语句，层号减1
    //删除该作用域中的符号
    symbolTable.index = symbolScopeStack.scopeArray[--symbolScopeStack.top];
}

//局部变量定义列表 语义分析
void def_list(struct Node *T) {
    if (T->ptr[0]) {
        T->ptr[0]->offset = T->offset;
        semanticAnalysis(T->ptr[0]); //处理一个局部变量定义
        T->width = T->ptr[0]->width;
    }
    if (T->ptr[1]) {
        T->ptr[1]->offset = T->offset + T->ptr[0]->width;
        semanticAnalysis(T->ptr[1]); //处理剩下的局部变量定义
        T->width += T->ptr[1]->width;
    }
}

//局部变量定义语句 语义分析
void var_def(struct Node *T) {
    int ret, num, width;
    struct Node *T0;
    //确定变量序列各变量类型和宽度
    if (!strcmp(T->ptr[0]->type_id, "int")) {
        T->type = T->ptr[1]->type = INT;
        width = 4;
    }
    if (!strcmp(T->ptr[0]->type_id, "float")) {
        T->type = T->ptr[1]->type = FLOAT;
        width = 8;
    }
    if (!strcmp(T->ptr[0]->type_id, "char")) {
        T->type = T->ptr[1]->type = CHAR;
        width = 1;
    }
    //T0为变量名列表子树根指针，对ID、ASSIGNOP类结点在登记到符号表，作为局部变量
    T0 = T->ptr[1];
    num = 0;
    T0->offset = T->offset;
    T->width = 0;
    while (T0) {    //处理所有DEC_LIST结点
        num++;
        //设置第一结点属性      
        T0->ptr[0]->type = T0->type;    //类型属性向下传递
        T0->ptr[0]->offset = T0->offset;     //偏移量属性向下传递
        if (T0->ptr[0]->kind == ID) {
            ret = fillSymbolTable(T0->ptr[0]->type_id, newAlias(), LEV,
                                  T0->ptr[0]->type, 'V', T->offset + T->width); 
            if (ret == -1) {
                semanticError(T0->ptr[0]->pos, T0->ptr[0]->type_id,
                               "变量重复定义");
            }
            else
                T0->ptr[0]->place = ret;
            T0->ptr[0]->width = width;
            // T->width += width;
        }
        else if (T0->ptr[0]->kind == ASSIGNOP) {
            ret = fillSymbolTable(T0->ptr[0]->ptr[0]->type_id, newAlias(),
                                  LEV, T0->ptr[0]->type, 'V', 
                                  T->offset + T->width);
            if (ret == -1) {
                semanticError(T0->ptr[0]->ptr[0]->pos,
                               T0->ptr[0]->ptr[0]->type_id, "变量重复定义");
            }
            else {
                T0->ptr[0]->place = ret;
                T0->ptr[0]->ptr[1]->offset = T->offset + T->width + width;
                Exp(T0->ptr[0]->ptr[1]);
            }
            T0->ptr[0]->width =  width + T0->ptr[0]->ptr[1]->width;
        }
        T->width += T0->ptr[0]->width;
        //设置第二结点属性
        if (T0->ptr[1]) {
            T0->ptr[1]->type = T0->type;
            T0->ptr[1]->offset = T0->offset + T0->ptr[0]->width;
        }
        T0 = T0->ptr[1];
    }
}

//语句列表 语义分析
void stm_list(struct Node *T) {
    //空语句；
    if (!T->ptr[0]) {
        T->width = 0;
        return;
    }
    T->ptr[0]->offset=T->offset;
    semanticAnalysis(T->ptr[0]);
    T->width = T->ptr[0]->width;
    if (T->ptr[1]) {
        strcpy(T->ptr[1]->sNext, T->sNext);
        T->ptr[1]->offset = T->offset; //顺序结构共享单元方式
        semanticAnalysis(T->ptr[1]);
        if (T->ptr[1]->width > T->width)
            T->width = T->ptr[1]->width; //顺序结构共享单元方式
    }
}

//if语句 语义分析
void if_then(struct Node *T) {
    T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
    boolExp(T->ptr[0]);     //if条件表达式
    T->width = T->ptr[0]->width;
    semanticAnalysis(T->ptr[1]);    //if子句
    if (T->width < T->ptr[1]->width)
        T->width = T->ptr[1]->width;
}

//if-else 语义分析
void if_then_else(struct Node *T) {
    T->ptr[0]->offset = T->ptr[1]->offset = T->ptr[2]->offset = T->offset;
    boolExp(T->ptr[0]);     //条件语句，要单独按短路代码处理
    T->width = T->ptr[0]->width;
    semanticAnalysis(T->ptr[1]);    //if子句
    if (T->width < T->ptr[1]->width)
        T->width = T->ptr[1]->width;
    semanticAnalysis(T->ptr[2]);    //else子句
    if (T->width < T->ptr[2]->width)
        T->width = T->ptr[2]->width;
}

//while 语义分析
void while_dec(struct Node *T) {
    T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
    boolExp(T->ptr[0]);     //循环条件，要单独按短路代码处理
    T->width = T->ptr[0]->width;
    ++is_loop;
    semanticAnalysis(T->ptr[1]); //循环体
    --is_loop;
    if (T->width < T->ptr[1]->width)
        T->width = T->ptr[1]->width;
}

//表达式语句 语义分析
void exp_stmt(struct Node *T) {
    T->ptr[0]->offset = T->offset;
    semanticAnalysis(T->ptr[0]);
    T->width = T->ptr[0]->width;
}

//return 语义分析
void return_dec(struct Node *T) {
    int ret, num;
    // struct OperandStruct opn1, opn2, result;
    if (T->ptr[0]) {
        T->ptr[0]->offset = T->offset;
        Exp(T->ptr[0]);
        //寻找函数定义确定返回值类型
        num = symbolTable.index;
        do{
            --num;
        }while (symbolTable.symbols[num].flag != 'F');
        if (T->ptr[0]->type != symbolTable.symbols[num].type) {
            semanticError(T->pos, "", "返回值类型错误");
            T->width = 0;
            return;
        }
        T->width = T->ptr[0]->width;
        need_return = 0;
    }
    else {
         //寻找函数定义确定返回值类型
        num = symbolTable.index;
        do{
            --num;
        }while (symbolTable.symbols[num].flag != 'F');
        if (symbolTable.symbols[num].type != VOID) {
            semanticError(T->pos, "", "返回值不能为空");
            T->width = 0;
            return;
        }
        T->width = 0;
    }
}

void break_exp(struct Node *T) {
    if (!is_loop)
        semanticError(T->pos, "", "不在循环体内部，不能使用break");
}

void continue_exp(struct Node *T) {
    if (!is_loop)
        semanticError(T->pos, "", "不在循环体内部，不能使用continue");
}