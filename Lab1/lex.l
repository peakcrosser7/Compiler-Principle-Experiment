/*声明部分*/
%{
    /*头文件声明*/
    #include <string.h>
    #include "parser.tab.h"
    #include "def.h"
    int yycolumn = 1;
    #define YY_USER_ACTION 	yylloc.first_line=yylloc.last_line=yylineno; \ 
	yylloc.first_column=yycolumn; \
	yylloc.last_column=yycolumn+yyleng-1; \
	yycolumn+=yyleng;
    /*语义值定义*/
    typedef union{
        int type_int;
        float type_float;
        char type_char;
        char type_id[32];
        struct Node *ptr;
    }YYLVAL;
    #define YYSTYPE YYLVAL
%}
%option yylineno  
/*辅助定义*/
id [A-Za-z][A-Za-z0-9]*
int [0-9]+
float ([0-9]*\.[0-9]+)|([0-9]+\.)
char \'[0-9aa-zA-z]\'

/* 规则部分：模式+动作 */
%%
{int} {yylval.type_int=atoi(yytext);return INT;}
{float} {yylval.type_float=atof(yytext); return FLOAT;}
{char} {yylval.type_char=yytext[1];return CHAR;}
"int" {strcpy(yylval.type_id,yytext); return TYPE;}
"float" {strcpy(yylval.type_id,yytext); return TYPE;}
"char" {strcpy(yylval.type_id,yytext); return TYPE;}
"return" {return RETURN;}
"if" {return IF;}
"else" {return ELSE;}
"while" {return WHILE;}
"for" {return FOR;}
"break" {return BREAK;}
"continue" {return CONTINUE;}
"void" {return VOID;}

{id} {strcpy(yylval.type_id,yytext); return ID;}
";" {return SEMI;}
"," {return COMMA;}
">"|"<"|">="|"<="|"=="|"!=" {strcpy(yylval.type_id,yytext); return RELOP;}
"=" {return ASSIGNOP;}
"+" {return PLUS;}
"-" {return MINUS;}
"+=" {return COMADD;}
"-=" {return COMSUB;}
"*=" {return COMSTAR;}
"/=" {return COMDIV;}
"++" {return SELFADD;}
"--" {return SELFSUB;}
"*" {return STAR;}
"/" {return DIV;}
"&&" {return AND;}
"||" {return OR;}
"!" {return NOT;}
"(" {return LP;}
")" {return RP;}
"[" {return LB;}
"]" {return RB;}
"{" {return LC;}
"}" {return RC;}
[\n] {yycolumn=1;}
[ \r\t] {;}
\/\/[^\n]*  {;}		//匹配单行注释的正则表达式
\/\*(\s|.)*?\*\/ {;}	//匹配多行注释的正则表达式
.   {printf("Error: Mysterious character\"%s\" at line %d,column %d\n",yytext,yylineno,yycolumn);}  //错误处理


%%
 /*C代码部分*/
 /*
void main(int argc,char *argv[]) {
    yylex();
    return;
}
*/
int yywrap() {
    return 1;
}


