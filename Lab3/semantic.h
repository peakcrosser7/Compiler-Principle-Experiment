#include "def.h"

extern int LEV;       //层号
extern int func_size; //1个函数的活动记录大小
extern int is_loop;
extern int need_return;
extern char continue_label[15];    //循环标号
extern char break_label[15];   //break标号

// semanticAnalysis.c
char *strcpycat(char *s1, char *s2);
char *newAlias();
char *newTemp();
void Exp(struct Node *T);
void semanticAnalysis(struct Node *T);
void semanticAnalysisInit(struct Node *T);
void semanticError(int line, char *msg1, char *msg2);
int matchParam(int i, struct Node *T);
void printSymbol();
int searchSymbolTable(char *name);
int fillSymbolTable(char *name, char *alias, int level, 
                    int type, char flag, int offset);
int fillTemp(char *name, int level, int type, char flag, int offset);

void printIR(struct CodeNode *head);
struct CodeNode *merge(int num,...);
char *newLabel();
struct CodeNode *genLabel(char *label);
struct CodeNode *genGoto(char *label);
struct CodeNode *genIR(int op, struct Operand opn1, struct Operand opn2,
                       struct Operand result);

// expCases.c
void boolExp(struct Node *T);
void id_exp(struct Node *T);
void int_exp(struct Node *T);
void float_exp(struct Node *T);
void char_exp(struct Node *T);
void assignop_exp(struct Node *T);
void relop_exp(struct Node *T);
void args_exp(struct Node *T);
void op_exp(struct Node *T);
void func_call_exp(struct Node *T);
void not_exp(struct Node *T);
void uminus_exp(struct Node *T);
void self_exp(struct Node *T);
void and_exp(struct Node *T);
void or_exp(struct Node *T);
void void_exp(struct Node *T);
void com_exp(struct Node *T);

// semanticCases.c
void ext_var_list(struct Node *T);
void ext_def_list(struct Node *T);
void ext_var_def(struct Node *T);
void func_def(struct Node *T);
void func_dec(struct Node *T);
void param_list(struct Node *T);
void param_dec(struct Node *T);
void comp_stm(struct Node *T);
void def_list(struct Node *T);
void var_def(struct Node *T);
void stm_list(struct Node *T);
void if_then(struct Node *T);
void if_then_else(struct Node *T);
void while_dec(struct Node *T);
void exp_stmt(struct Node *T);
void return_dec(struct Node *T);
void break_exp(struct Node *T);
void continue_exp(struct Node *T);