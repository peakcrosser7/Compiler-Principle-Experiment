/* 处理 emanticAnalysis 中的switch case */
#include "semantic.h"


//外部定义列表 语义分析
void ext_def_list(struct Node *T) {
    if (!T->ptr[0])     //第一个外部定义语句
        return;
    T->ptr[0]->offset = T->offset;   // 语义分析之前先设置偏移地址
    semanticAnalysis(T->ptr[0]); //访问外部定义列表中的第一个
    T->code = T->ptr[0]->code;  //合并code
    // 其他外部定义语句，可为空
    if (T->ptr[1]) {
        T->ptr[1]->offset = T->ptr[0]->offset + T->ptr[0]->width;
        semanticAnalysis(T->ptr[1]); //访问该外部定义列表中的其它外部定义
        T->code = merge(2, T->code, T->ptr[1]->code);   //合并中间代码
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
    T->code = NULL;     //这里假定外部变量不支持初始化，不生成中间代码
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

    strcpy(T->ptr[2]->sNext, newLabel());   //函数出口标号，即函数体语句执行后位置
    
    semanticAnalysis(T->ptr[2]);            //处理函数体结点
    if(need_return){
         semanticError(T->pos, T->type_id, "函数缺少返回值");
    }
    //计算活动记录大小:这里offset属性存放的是活动记录大小，不是偏移
    symbolTable.symbols[T->ptr[1]->place].offset
        = T->offset + T->ptr[2]->width;
    //将函数定义、函数体和函数出口的中间代码合并
    T->code = merge(3, T->ptr[1]->code, T->ptr[2]->code,
                    genLabel(T->ptr[2]->sNext));
}

//函数定义 语义分析
void func_dec(struct Node *T) {
    int ret;
    struct Operand opn1, opn2, result;
    //函数不在数据区中分配单元，偏移量为0
    ret = fillSymbolTable(T->type_id, newAlias(), LEV, T->type, 'F', 0);
    if (ret == -1) {
        semanticError(T->pos, T->type_id, "函数重复定义");
        return;
    }
    else T->place = ret;
    //result存放函数信息
    result.kind = ID;   //运算结果类型为 标识符（函数）
    strcpy(result.id, T->type_id);  //运算结果名为 函数名
    result.offset = ret;    //运算结果的偏移量为 函数在符号表位置
    //生成中间代码：FUNCTION 函数名
    T->code = genIR(FUNCTION, opn1, opn2, result);

    T->offset = DX;     //设置形式参数在活动记录中的偏移量初值
    if (T->ptr[0]) {    //函数有形参
        T->ptr[0]->offset = T->offset;
        semanticAnalysis(T->ptr[0]);    //处理函数参数列表
        T->width = T->ptr[0]->width;
        symbolTable.symbols[ret].paramnum = T->ptr[0]->num; //形参个数
        //合并函数名和形参中间代码序列
        T->code = merge(2, T->code, T->ptr[0]->code);
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
        T->num = T->ptr[0]->num + T->ptr[1]->num;       //统计参数个数
        T->width = T->ptr[0]->width + T->ptr[1]->width; //累加参数单元宽度
        T->code = merge(2, T->ptr[0]->code, T->ptr[1]->code); //合并形参中间代码
    }
    else {  //形参只有一个
        T->num = T->ptr[0]->num;
        T->width = T->ptr[0]->width;
        T->code = T->ptr[0]->code;  //第一个形参的中间代码即为形参列表的中间代码
    }
}

//函数形参定义 语义分析
void param_dec(struct Node *T) {
    int ret;
    struct Operand opn1, opn2, result;
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

    result.kind = ID;   //运算结果类型为 标识符（形参）
    strcpy(result.id, symbolTable.symbols[ret].alias);  //名称用形参别名
    result.offset = T->offset;  //形参在符号表位置（使用offset以确保出函数后的可见性消失）
    //生成中间代码：PARAM 形参名
    T->code = genIR(PARAM, opn1, opn2, result);
}

//复合语句 语义分析
void comp_stm(struct Node *T) {
    LEV++;
    //设置层号加1，并且保存该层局部变量在符号表中的起始位置在symbolScopeArray
    symbolScopeStack.scopeArray[symbolScopeStack.top++] = symbolTable.index;
    T->width = 0;
    T->code = NULL;

    if (T->ptr[0]) {
        T->ptr[0]->offset = T->offset;
        semanticAnalysis(T->ptr[0]);    //处理该层的局部变量DEF_LIST
        T->width += T->ptr[0]->width;
        T->code = T->ptr[0]->code;
    }
    if (T->ptr[1]) {
        T->ptr[1]->offset = T->offset + T->width;
        strcpy(T->ptr[1]->sNext, T->sNext); //S.next属性向下传递
        semanticAnalysis(T->ptr[1]);       //处理复合语句的语句序列
        T->width += T->ptr[1]->width;
        T->code = merge(2, T->code, T->ptr[1]->code);   //合并复合语句中间代码
    }

    printSymbol();  //在退出一个符合语句前显示的符号表
    // putchar('\n');
    LEV--;          //出复合语句，层号减1
    //删除该作用域中的符号
    symbolTable.index = symbolScopeStack.scopeArray[--symbolScopeStack.top];
}

//局部变量定义列表 语义分析
void def_list(struct Node *T) {
    T->code = NULL;

    if (T->ptr[0]) {
        T->ptr[0]->offset = T->offset;
        semanticAnalysis(T->ptr[0]); //处理一个局部变量定义
        T->width = T->ptr[0]->width;
        T->code = T->ptr[0]->code;
    }
    if (T->ptr[1]) {
        T->ptr[1]->offset = T->offset + T->ptr[0]->width;
        semanticAnalysis(T->ptr[1]); //处理剩下的局部变量定义
        T->width += T->ptr[1]->width;
        //合并多个变量定义的中间代码
        T->code = merge(2, T->code, T->ptr[1]->code);
    }
}

//局部变量定义语句 语义分析
void var_def(struct Node *T) {
    int ret, num, width;
    struct Node *T0;
    struct Operand opn1, opn2, result;
    T->code = NULL;
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

                opn1.kind = ID; //操作数1类型为 标识符
                //操作数1名为赋值表达式的符号的别名
                strcpy(opn1.id,
                       symbolTable.symbols[T0->ptr[0]->ptr[1]->place].alias);
                opn1.offset = symbolTable.symbols[T0->ptr[0]->ptr[1]->place].offset;
                result.kind = ID;
                //运算结果名为局部变量名
                strcpy(result.id,
                       symbolTable.symbols[T0->ptr[0]->place].alias);
                result.offset = symbolTable.symbols[T0->ptr[0]->place].offset;
                //生成赋值语句中间代码，并与前面的变量定义代码和赋值号右边表达式的中间代码合并
                T->code = merge(3, T->code, T0->ptr[0]->ptr[1]->code,
                                genIR(ASSIGNOP, opn1, opn2, result));
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
    //2条以上语句连接，生成新标号作为第一条语句出口
    if (T->ptr[1])
        strcpy(T->ptr[0]->sNext, newLabel());
    //语句序列仅有一条语句，语句列表出口即为语句出口
    else 
        strcpy(T->ptr[0]->sNext, T->sNext);
    
    T->ptr[0]->offset=T->offset;
    semanticAnalysis(T->ptr[0]);
    T->code = T->ptr[0]->code;
    T->width = T->ptr[0]->width;
    
    if (T->ptr[1]) {
         //2条以上语句连接,语句列表出口为第2结点的出口
        strcpy(T->ptr[1]->sNext, T->sNext);
        T->ptr[1]->offset = T->offset; //顺序结构共享单元方式
        semanticAnalysis(T->ptr[1]);
        //序列中第1条为表达式语句，返回语句，复合语句时，第2条前不需要标号
        if (T->ptr[0]->kind == RETURN || T->ptr[0]->kind == RETURN_E 
        || T->ptr[0]->kind == EXP_STMT || T->ptr[0]->kind == COMP_STM) {
            //合并语句列表和当前语句的中间代码
            T->code = merge(2, T->code, T->ptr[1]->code);
        }
        else{
            //生成第1条出口的中间代码，并与句列表和当前语句的中间代码合并
            T->code = merge(3, T->code, genLabel(T->ptr[0]->sNext),
                            T->ptr[1]->code);
        }
        if (T->ptr[1]->width > T->width)
            T->width = T->ptr[1]->width; //顺序结构共享单元方式
    }
}

//if语句 语义分析
void if_then(struct Node *T) {
    //设置条件语句真假出口
    strcpy(T->ptr[0]->eTrue, newLabel());   //真出口是（if子句）新标号
    strcpy(T->ptr[0]->eFalse, T->sNext);    //假出口是if语句出口
    T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
    boolExp(T->ptr[0]);     //if条件表达式
    T->width = T->ptr[0]->width;

    strcpy(T->ptr[1]->sNext, T->sNext);     //if子句出口为if语句出口
    semanticAnalysis(T->ptr[1]);    //if子句
    if (T->width < T->ptr[1]->width)
        T->width = T->ptr[1]->width;
    //合并条件语句、if子句标号、if子句的中间代码
    T->code = merge(3, T->ptr[0]->code, genLabel(T->ptr[0]->eTrue),
                    T->ptr[1]->code);
}

//if-else 语义分析
void if_then_else(struct Node *T) {
    //设置条件语句真假出口均为新标号
    strcpy(T->ptr[0]->eTrue, newLabel());   //if子句标号
    strcpy(T->ptr[0]->eFalse, newLabel());  //else子句标号
    T->ptr[0]->offset = T->ptr[1]->offset = T->ptr[2]->offset = T->offset;
    boolExp(T->ptr[0]);     //条件语句，要单独按短路代码处理
    T->width = T->ptr[0]->width;

    strcpy(T->ptr[1]->sNext, T->sNext); //if子句出口为if语句出口
    semanticAnalysis(T->ptr[1]);    //if子句
    if (T->width < T->ptr[1]->width)
        T->width = T->ptr[1]->width;

    strcpy(T->ptr[2]->sNext, T->sNext); //else子句出口为if语句出口
    semanticAnalysis(T->ptr[2]);    //else子句
    if (T->width < T->ptr[2]->width)
        T->width = T->ptr[2]->width;
    //合并条件语句、if子句标号、if子句、跳转到if语句出口、else子句标号、else子句的中间代码
    T->code = merge(6, T->ptr[0]->code, genLabel(T->ptr[0]->eTrue), T->ptr[1]->code,
                    genGoto(T->sNext), genLabel(T->ptr[0]->eFalse), T->ptr[2]->code);
}

//while 语义分析
void while_dec(struct Node *T) {
    strcpy(T->ptr[0]->eTrue, newLabel());   //循环条件真出口为（循环体）新标号
    strcpy(T->ptr[0]->eFalse, T->sNext);    //循环条件假出口为循环语句出口
    T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
    boolExp(T->ptr[0]);     //循环条件，要单独按短路代码处理
    T->width = T->ptr[0]->width;
    strcpy(T->ptr[1]->sNext, newLabel());   //循环体出口为（循环条件）新标号
    strcpy(continue_label, T->ptr[1]->sNext);   //记录循环条件的标号，用于continue
    strcpy(break_label, T->ptr[0]->eFalse); //维护循环条件假出口的标号，用于break
    ++is_loop;
    semanticAnalysis(T->ptr[1]); //循环体
    --is_loop;
    if (T->width < T->ptr[1]->width)
        T->width = T->ptr[1]->width;
    //合并循环条件标号、循环条件、循环体标号、循环体、跳转到循环条件标号的中间代码
    T->code = merge(5, genLabel(T->ptr[1]->sNext), T->ptr[0]->code,
                    genLabel(T->ptr[0]->eTrue), T->ptr[1]->code,
                    genGoto(T->ptr[1]->sNext));
}

//表达式语句 语义分析
void exp_stmt(struct Node *T) {
    T->ptr[0]->offset = T->offset;
    semanticAnalysis(T->ptr[0]);
    T->width = T->ptr[0]->width;
    //表达式的中间代码即为表达式语句的中间代码
    T->code = T->ptr[0]->code;
}

//return 语义分析 --waitchg返回后退出函数
void return_dec(struct Node *T) {
    int ret, num;
    struct Operand opn1, opn2, result;
    //有返回值
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
            T->code = NULL;
            return;
        }
        T->width = T->ptr[0]->width;
        need_return = 0;
        result.kind = ID;   //运算结果
        //运算结果名为返回值的符号别名
        strcpy(result.id, symbolTable.symbols[T->ptr[0]->place].alias);
        //运算结果位置
        result.offset = symbolTable.symbols[T->ptr[0]->place].offset;
        //生成RETURN中间代码，并与返回值中间代码合并
        T->code = merge(2, T->ptr[0]->code, genIR(RETURN, opn1, opn2, result));
    }
    //无返回值
    else {
         //寻找函数定义确定返回值类型
        num = symbolTable.index;
        do{
            --num;
        }while (symbolTable.symbols[num].flag != 'F');
        if (symbolTable.symbols[num].type != VOID) {
            semanticError(T->pos, "", "返回值不能为空");
            T->width = 0;
            T->code = NULL;
            return;
        }
        T->width = 0;
        //生成RETURN的中间代码
        result.kind = VOID;
        T->code = genIR(RETURN_E, opn1, opn2, result);
    }
}

void break_exp(struct Node *T) {
    if (!is_loop)
        semanticError(T->pos, "", "不在循环体内部，不能使用break");
    else
        //跳转到循环条件假出口标号
        T->code = genGoto(break_label);
}

void continue_exp(struct Node *T) {
    if (!is_loop)
        semanticError(T->pos, "", "不在循环体内部，不能使用continue");
    else
        //跳转到循环条件标号
        T->code = genGoto(continue_label);
}