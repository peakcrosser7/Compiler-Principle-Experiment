%error-verbose	//指示bison生成错误信息
%locations		//记录行号

//声明部分
%{
#include "stdio.h"
#include "math.h"
#include "string.h"
#include "def.h"
extern int yylineno;
extern char* yytext;
extern FILE* yyin;
void yyerror(const char* fmt, ...);
void displayAST(struct Node* , int);
%}

//声明各种类型, 默认为int
%union {
	int type_int;
	float type_float;
	char type_char;
	char type_id[32];
	struct Node* ptr;
};

//%type指定非终结符的语义值类型 %type <union的成员名> 非终结符名
%type<ptr> Program ExtDefList ExtDef Specifier ExtDecList FuncDec CompSt VarList VarDec ParamDec Stmt StmList DefList Def DecList Dec Exp ExpList
//%token指定终结符的语义值类型
%token<type_int> INT
%token<type_id> ID RELOP TYPE
%token<type_float> FLOAT
%token<type_char> CHAR
%token LP RP LC RC SEMI COMMA LB RB
%token PLUS MINUS STAR DIV ASSIGNOP AND OR NOT IF ELSE WHILE RETURN COMADD COMSUB COMSTAR COMDIV BREAK CONTINUE VOID
//结合性，优先级由低到高
%left COMADD COMSUB COMSTAR COMDIV
%left ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right UMINUS NOT SELFADD SELFSUB
//无结合性
%nonassoc LOWER_THEN_ELSE
%nonassoc ELSE

%%
//整个程序: 语义分析入口,显示整棵抽象语法树
Program: ExtDefList {
	// displayAST($1,0);
	semanticAnalysisInit($1);
	}	
	;
//外部定义列表: 即整棵抽象语法树,每个结点第1棵子树对应外部变量声明或函数
ExtDefList: {$$=NULL;}	//抽象语法树为空
	| ExtDef ExtDefList {$$=mknode(EXT_DEF_LIST,yylineno,2,$1,$2);}	//其第1棵子树对应一个外部变量或函数定义
	;
//外部定义语句
ExtDef: Specifier ExtDecList SEMI {$$=mknode(EXT_VAR_DEF,yylineno,2,$1,$2);}		//外部(全局)变量定义语句
	| Specifier FuncDec CompSt {$$=mknode(FUNC_DEF,yylineno,3,$1,$2,$3);}		//函数定义
	| error SEMI {$$=NULL;}
	;
//变量类型: 包括int,float,char
Specifier: TYPE {$$=mknode(TYPE,yylineno,0); strcpy($$->type_id,$1); $$->type=(!strcmp($1,"int")?INT:(!strcmp($1,"float")?FLOAT:CHAR));}
	| VOID {$$=mknode(VOID,yylineno,0); strcpy($$->type_id,"void"); $$->type=VOID;}
	;
//变量名列表：由一个或多个变量名组成，多个变量名之间用逗号隔开
//其第1棵子树对应一个变量名(ID类型的结点)，第2棵子树对应剩下的外部变量名
ExtDecList: VarDec {$$=$1;}
	| VarDec COMMA ExtDecList {$$=mknode(EXT_DEC_LIST,yylineno,2,$1,$3);}
	;
//变量名: 由一个标识符ID或数组名ID组成
VarDec: ID {$$=mknode(ID,yylineno,0);strcpy($$->type_id,$1);}	//ID结点,标识符字符串存放结点的type_id中
	// | VarDec LB Exp RB {$$=mknode(ARRAY_DEC, yylineno, 2, $1, $3);}		//数组结点
	;
//函数定义
FuncDec: ID LP VarList RP {$$=mknode(FUNC_DEC,yylineno,1,$3);strcpy($$->type_id,$1);}	//有入口参数
	| ID LP RP {$$=mknode(FUNC_DEC,yylineno,0);strcpy($$->type_id,$1);}	//无入口参数
	| error RP {$$=NULL; printf("---函数定义存在错误---\n");}
	;
//函数入口参数列表: 由一到多个函数参数定义组成，用逗号隔开
VarList: ParamDec {$$=mknode(PARAM_LIST,yylineno,1,$1);}
	| ParamDec COMMA VarList {$$=mknode(PARAM_LIST,yylineno,2,$1,$3);}
	;
//函数参数定义:由1个变量类型和1个变量名组成
ParamDec: Specifier VarDec {$$=mknode(PARAM_DEC,yylineno,2,$1,$2);}
	;
//复合语句
CompSt: LC DefList StmList RC {$$=mknode(COMP_STM,yylineno,2,$2,$3);}
	| error RC {$$=NULL; printf("---复合语句存在错误---\n");}
	;
//语句列表: 由0个或多个语句组成
StmList: {$$=NULL;}
	| Stmt StmList {$$=mknode(STM_LIST,yylineno,2,$1,$2);}
	;
//语句: 可为表达式,复合语句,return语句,if语句,if-else语句,while语句,for语句
Stmt: Exp SEMI {$$=mknode(EXP_STMT,yylineno,1,$1);}
	| CompSt {$$=$1;}	//复合语句结点直接作为语句结点,不再生成新的结点
	| RETURN Exp SEMI {$$=mknode(RETURN,yylineno,1,$2);}
	| IF LP Exp RP Stmt %prec LOWER_THEN_ELSE {$$=mknode(IF_THEN,yylineno,2,$3,$5);}	//if语句
	| IF LP Exp RP Stmt ELSE Stmt {$$=mknode(IF_THEN_ELSE,yylineno,3,$3,$5,$7);}	//if-else语句
	| WHILE LP Exp RP Stmt {$$=mknode(WHILE,yylineno,2,$3,$5);}
	| BREAK SEMI {$$=mknode(BREAK,yylineno,0);strcpy($$->type_id,"BREAK");}
	| CONTINUE SEMI {$$=mknode(CONTINUE,yylineno,0);strcpy($$->type_id,"CONTINUE");}
	| RETURN SEMI {$$=mknode(RETURN_E,yylineno,0);strcpy($$->type_id,"RETURN");}
	| SEMI {$$=NULL;}
	;
//定义列表: 由0个或多个定义语句组成
DefList: {$$=NULL; }
	| Def DefList {$$=mknode(DEF_LIST,yylineno,2,$1,$2);}
	;
//定义语句: 由分号隔开
Def: Specifier DecList SEMI {$$=mknode(VAR_DEF,yylineno,2,$1,$2);}	//不同变量定义
     ;
//变量列表，由一个或多个变量语句组成，由逗号隔开
DecList: Dec {$$=mknode(DEC_LIST,yylineno,1,$1);}
	| Dec COMMA DecList {$$=mknode(DEC_LIST,yylineno,2,$1,$3);}
	;
//变量语句: 1个变量名或者1个变量赋值语句
Dec: VarDec {$$=$1;}
	| VarDec ASSIGNOP Exp {$$=mknode(ASSIGNOP,yylineno,2,$1,$3);strcpy($$->type_id,"ASSIGNOP");}
	;
//表达式
Exp: Exp ASSIGNOP Exp {$$=mknode(ASSIGNOP,yylineno,2,$1,$3);strcpy($$->type_id,"ASSIGNOP");}//type_id记录运算符
	| Exp AND Exp {$$=mknode(AND,yylineno,2,$1,$3);strcpy($$->type_id,"AND");}
	| Exp OR Exp {$$=mknode(OR,yylineno,2,$1,$3);strcpy($$->type_id,"OR");}
	| Exp RELOP Exp {$$=mknode(RELOP,yylineno,2,$1,$3);strcpy($$->type_id,$2);}//词法分析关系运算符号自身值保存在$2中
	| Exp PLUS Exp {$$=mknode(PLUS,yylineno,2,$1,$3);strcpy($$->type_id,"PLUS");}
	| Exp MINUS Exp {$$=mknode(MINUS,yylineno,2,$1,$3);strcpy($$->type_id,"MINUS");}
	| Exp STAR Exp {$$=mknode(STAR,yylineno,2,$1,$3);strcpy($$->type_id,"STAR");}
	| Exp DIV Exp {$$=mknode(DIV,yylineno,2,$1,$3);strcpy($$->type_id,"DIV");}
	| Exp COMADD Exp {$$=mknode(COMADD,yylineno,2,$1,$3);strcpy($$->type_id,"COMADD");}
	| Exp COMSUB Exp {$$=mknode(COMSUB,yylineno,2,$1,$3);strcpy($$->type_id,"COMSUB");}
	| Exp COMSTAR Exp {$$=mknode(COMSTAR,yylineno,2,$1,$3);strcpy($$->type_id,"COMSTAR");}
	| Exp COMDIV Exp {$$=mknode(COMDIV,yylineno,2,$1,$3);strcpy($$->type_id,"COMDIV");}
	| LP Exp RP {$$=$2;}//遇到左右括号,直接忽略括号,表达式的值就为括号里面的表达式值
	| MINUS Exp %prec UMINUS {$$=mknode(UMINUS,yylineno,1,$2);strcpy($$->type_id,"UMINUS");}	//负号
	| NOT Exp {$$=mknode(NOT,yylineno,1,$2);strcpy($$->type_id,"NOT");}
	| Exp SELFADD {$$=mknode(SELFADD_R,yylineno,1,$1);strcpy($$->type_id,"SELFADD");}
	| Exp SELFSUB {$$=mknode(SELFSUB_R,yylineno,1,$1);strcpy($$->type_id,"SELFSUB");}
	| ID LP ExpList RP {$$=mknode(FUNC_CALL,yylineno,1,$3);strcpy($$->type_id,$1);}	//有参数函数调用
	| ID LP RP {$$=mknode(FUNC_CALL,yylineno,0);strcpy($$->type_id,$1);}	//无参数函数调用
	| ID {$$=mknode(ID,yylineno,0);strcpy($$->type_id,$1);}
	| INT {$$=mknode(INT,yylineno,0);$$->type_int=$1;$$->type=INT;}
	| FLOAT {$$=mknode(FLOAT,yylineno,0);$$->type_float=$1;$$->type=FLOAT;}
	| CHAR {$$=mknode(CHAR,yylineno,0); $$->type_char=$1;$$->type=CHAR;}
	;
//实参列表
ExpList: Exp COMMA ExpList {$$=mknode(EXP_LIST,yylineno,2,$1,$3);}
	| Exp {$$=mknode(EXP_LIST,yylineno,1,$1);}
	;

%%

int main(int argc, char *argv[]){
  yyin=fopen(argv[1],"r");
  if (!yyin) 
    return 0;
  yylineno=1;
  yyparse();
  return 0;
}

#include<stdarg.h>
void yyerror(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "Grammar Error at Line %d Column %d: ", yylloc.first_line,yylloc.first_column);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, ".\n");
}





