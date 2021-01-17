/* 数据结构定义*/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"
#include "parser.tab.h"

enum NodeKind { EXT_DEF_LIST,EXT_VAR_DEF,FUNC_DEF,FUNC_DEC,EXT_DEC_LIST,
                PARAM_LIST,PARAM_DEC,VAR_DEF,DEC_LIST,DEF_LIST,COMP_STM,
                STM_LIST,EXP_STMT,IF_THEN,IF_THEN_ELSE,FUNC_CALL,EXP_LIST,FUNCTION,
                PARAM,ARG,CALL,LABEL,GOTO,JLT,JLE,JGT,JGE,EQ,NEQ,
                SELFADD_L,SELFADD_R,SELFSUB_L,SELFSUB_R
              };
#define MAXLENGTH 1000     //定义符号表的大小
#define DX 3 * sizeof(int) //活动记录控制信息需要的单元数

//操作数
struct OperandStruct {
  int kind; //标识操作的类型
  int type; //标识操作数的类型
  union {
    int const_int;     //整常数值,立即数
    float const_float; //浮点常数值,立即数
    char const_char;   //字符常数值,立即数
    char id[33];       //变量或临时变量的别名或标号字符串
  };
  int level;  //变量的层号,0表示是全局变量,数据保存在静态数据区
  int offset; //变量单元偏移量,或函数在符号表的定义位置序号,目标代码生成时用
};

struct CodeNode { //三地址TAC代码结点,采用双向循环链表存放中间语言代码
  int op;                                  //TAC代码的运算符种类
  struct OperandStruct opn1, opn2, result; //2个操作数和运算结果
  struct CodeNode *next, *prior;
};

struct Node { //以下对结点属性定义没有考虑存储效率,只是简单地列出要用到的一些属性
  enum NodeKind kind; //结点类型
  union {
    char type_id[33]; //由标识符生成的叶结点
    int type_int;     //由整常数生成的叶结点
    char type_char;   //由字符型生成的叶节点
    float type_float; //由浮点常数生成的叶结点
  };
  struct Node *ptr[4];        //子树指针,由kind确定有多少棵子树
  int level;                  //层号
  int place;                  //表示结点对应的变量或运算结果临时变量在符号表的位置序号
  char eTrue[15], eFalse[15]; //对布尔表达式的翻译时,真假转移目标的标号
  char sNext[15];             //该结点对应语句执行后的下一条语句位置标号
  struct CodeNode *code;      //该结点中间代码链表头指针
  char op[10];
  int type;   //结点对应值的类型
  int pos;    //语法单位所在位置行号
  int offset; //偏移量
  int width;  //各种数据占用的字节数
  int num;    //变量个数
};

//符号结构体
struct Symbol {   //这里只列出了一个符号表项的部分属性,没考虑属性间的互斥
  char name[33];  //变量或函数名
  int level;      //层号,外部变量名或函数名层号为0,形参名为1,每到1个复合语句层号加1,退出减1
  int type;       //变量类型或函数返回值类型
  int paramnum;   //形式参数个数
  char alias[10]; //别名,为解决嵌套层次使用,使得每一个数据名称唯一
  char flag;      //符号标记,函数:'F' 变量:'V' 参数:'P' 临时变量:'T'
  char offset;    //外部变量和局部变量在其静态数据区或活动记录中的偏移量 或 函数活动记录大小,目标代码生成时使用
};

//符号表,是一个顺序栈,index初值为0
struct SymbolTable {
  struct Symbol symbols[MAXLENGTH];
  int index;  //符号表索引
} symbolTable;

//作用域栈
struct SymbolScopeArray {
  int scopeArray[30];
  int top;
} symbolScopeStack;
/*当前作用域的符号在符号表的起始位置序号,栈结构,每到达一个复合语句,
将符号表的index值进栈,离开复合语句时,取其退栈值修改符号表的index值,
完成删除该复合语句中的所有变量和临时变量*/

struct Node *mknode(int kind, int pos, int num, ...);
void semanticAnalysisInit(struct Node *T);
void semanticAnalysis(struct Node *T);
void boolExp(struct Node *T);
void Exp(struct Node *T);
void objectCode(struct CodeNode *head);

