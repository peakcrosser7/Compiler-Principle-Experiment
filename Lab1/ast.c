#include "def.h"
#include <stdio.h>

//建立抽象语法树结点
struct Node *mknode(int kind,int pos,int num,...) {
	struct Node *T = (struct Node*)malloc(sizeof(struct Node));
	int i=0;
	T->kind = kind;	//类型
	T->pos = pos;	//位置
	va_list pArgs;	//变长参数
	va_start(pArgs,num);
	for(i=0;i<num;++i) T->ptr[i]=va_arg(pArgs, struct Node *);
	while(i<4) T->ptr[i++]=NULL;
	va_end(pArgs);
	return T;
}

//对抽象语法树先根遍历并显示
void displayAST(struct Node* T,int indent) {
    if(T){
        switch (T->kind){
            case EXT_DEF_LIST:  //外部定义列表
                displayAST(T->ptr[0],indent);	//显示该外部定义列表中的第一个
                displayAST(T->ptr[1],indent);   //显示外部定义列表中的其它外部定义
                break;
            case EXT_VAR_DEF:	//外部变量定义
                printf("%*c%s\n",indent,' ',"外部变量定义：");     
                displayAST(T->ptr[0],indent+3);       //显示外部变量类型
                printf("%*c%s\n",indent+3,' ',"变量名：");
                displayAST(T->ptr[1],indent+3);       //显示变量列表
                break;
            case FUNC_DEF:	//函数定义语句
                printf("%*c%s\n",indent,' ',"函数定义：");
                displayAST(T->ptr[0],indent+3);	//显示函数返回类型
                displayAST(T->ptr[1],indent+3); //显示函数名和参数
                displayAST(T->ptr[2],indent+3);	//显示函数体
                break;
            case ARRAY_DEF:	//数组定义语句
                printf("%*c%s\n",indent,' ',"数组定义：");
                displayAST(T->ptr[0],indent+3);
                displayAST(T->ptr[1],indent+3);
                break;
            case FUNC_DEC:	//函数定义
                printf("%*c%s%s\n",indent,' ',"函数名：",T->type_id);
		if(T->ptr[0]){
                	printf("%*c%s\n",indent,' ',"函数形参：");//显示函数参数列表
                	displayAST(T->ptr[0],indent+3);
		}
		else printf("%*c%s\n",indent,' ',"无形参");
                break;
            case ARRAY_DEC:	//数组定义
                printf("%*c%s%s\n",indent,' ',"数组名：",T->type_id);
                printf("%*c%s\n",indent,' ',"数组大小：");
                displayAST(T->ptr[0],indent+3);
                break;
            case EXT_DEC_LIST:	//变量名列表
                displayAST(T->ptr[0],indent+3);    //依次显示外部变量名
                if(T->ptr[1]->ptr[0]==NULL)//后续还有相同的，仅显示语法树此处
                    displayAST(T->ptr[1],indent+3);
                else
                    displayAST(T->ptr[1],indent);
                break;
            case PARAM_LIST:	//函数入口参数列表
                displayAST(T->ptr[0],indent);//依次显示全部参数类型和名称
                displayAST(T->ptr[1],indent);
                break;
            case PARAM_DEC:		//函数参数定义
                displayAST(T->ptr[0],indent);
                displayAST(T->ptr[1],indent);
                break;
            case VAR_DEF:	//变量定义
                displayAST(T->ptr[0],indent+3);   //显示变量类型
                displayAST(T->ptr[1],indent+3);  //显示该定义的全部变量名
                break;
            case DEC_LIST:	//变量列表
                printf("%*c%s\n",indent,' ',"变量名：");
                displayAST(T->ptr[0],indent+3);
                displayAST(T->ptr[1],indent);
                break;
            case DEF_LIST:	//定义列表
                printf("%*c%s\n",indent+3,' ',"局部变量名：");
		while(T){
                	displayAST(T->ptr[0],indent+3);
			T=T->ptr[1];
		}
                break;
            case COMP_STM:	//复合语句
                printf("%*c%s\n",indent,' ',"复合语句：");
                printf("%*c%s\n",indent+3,' ',"复合语句的变量定义：");
                displayAST(T->ptr[0],indent+3);	//显示定义部分
                printf("%*c%s\n",indent+3,' ',"复合语句的语句部分：");
                displayAST(T->ptr[1],indent+3);	//显示语句部分
                break;
            case STM_LIST:	//语句列表
                displayAST(T->ptr[0],indent+3);	//显示第一条语句
                displayAST(T->ptr[1],indent);	//显示剩下语句
                break;
            case EXP_STMT:	//表达式
                printf("%*c%s\n",indent,' ',"表达式语句：");
                displayAST(T->ptr[0],indent+3);
                break;
            case IF_THEN:	//if语句
                printf("%*c%s\n",indent,' ',"if条件语句：");
                printf("%*c%s\n",indent,' ',"条件：");
                displayAST(T->ptr[0],indent+3);     //显示条件
                printf("%*c%s\n",indent,' ',"if子句：");
                displayAST(T->ptr[1],indent+3);     //显示if子句
                break;
            case IF_THEN_ELSE:	//if-else if语句
                printf("%*c%s\n",indent,' ',"if-else条件语句：");
		printf("%*c%s\n",indent,' ',"条件：");
                displayAST(T->ptr[0],indent+3);     //显示条件
		printf("%*c%s\n",indent,' ',"if子句：");
                displayAST(T->ptr[1],indent+3);
		printf("%*c%s\n",indent,' ',"else子句：");
		displayAST(T->ptr[2],indent+3);
                break;
            case WHILE:	//while语句
                printf("%*c%s\n",indent,' ',"while循环语句：");
                printf("%*c%s\n",indent+3,' ',"循环条件：");
                displayAST(T->ptr[0],indent+3);     //显示循环条件
                printf("%*c%s\n",indent+3,' ',"循环体：");
                displayAST(T->ptr[1],indent+3);     //显示循环体
                break;
            case FOR:	//for语句
                printf("%*c%s\n",indent,' ',"for循环语句：");
                printf("%*c%s\n",indent+3,' ',"表达式1：");
                displayAST(T->ptr[0],indent+3);
                printf("%*c%s\n",indent+3,' ',"表达式2：");
                displayAST(T->ptr[1],indent+3);
		printf("%*c%s\n",indent+3,' ',"表达式3：");
		displayAST(T->ptr[2],indent+3);
		printf("%*c%s\n",indent+3,' ',"循环体：");
		displayAST(T->ptr[3],indent+3);
                break;
	    case FOR_E:
		printf("%*c%s\n",indent,' ',"for循环语句：");
		printf("%*c%s\n",indent+3,' ',"循环条件：");
		displayAST(T->ptr[0],indent+3);
		printf("%*c%s\n",indent+3,' ',"循环体：");
		displayAST(T->ptr[1],indent+3);
		break;
            case FUNC_CALL:	//函数调用语句
                printf("%*c%s\n",indent,' ',"函数调用：");
                printf("%*c%s%s\n",indent+3,' ',"函数名：",T->type_id);
                printf("%*c%s\n",indent+3,' ',"实参：");
                displayAST(T->ptr[0],indent+3);
                break;
            case EXP_LIST:	//实参列表
		while(T){
			displayAST(T->ptr[0],indent+3);
			T=T->ptr[1];		
		}
                break;
            case ID:	//标识符
                printf("%*c标识符： %s\n",indent,' ',T->type_id);//控制新的一行输出的空格数，indent代替%*c中*
                break;
            case INT:
                printf("%*cINT： %d\n",indent,' ',T->type_int);  
                break;
            case FLOAT:
                printf("%*cFLOAT： %f\n",indent,' ',T->type_float);  
                break;
            case CHAR:
                printf("%*cCHAR： %c\n",indent,' ',T->type_char);
            case ARRAY:
                printf("%*c数组名称： %s\n",indent,' ',T->type_id);  
                break;
            case TYPE:
                if(T->type==INT)
                    printf("%*c%s\n",indent,' ',"类型：int");
                else if(T->type==FLOAT)
                    printf("%*c%s\n",indent,' ',"类型：float");
                else if(T->type==CHAR)
                    printf("%*c%s\n",indent,' ',"类型：char");
                break;
            case ASSIGNOP:
            case OR:
            case SELFADD_L:
            case SELFSUB_L:
            case SELFADD_R:
            case SELFSUB_R:
            case AND:
            case RELOP:
            case PLUS:
            case MINUS:
            case STAR:
            case DIV:
            case COMADD:
            case COMSUB:
	    case COMSTAR:
	    case COMDIV:
                printf("%*c%s\n",indent,' ',T->type_id);
                displayAST(T->ptr[0],indent+3);
                displayAST(T->ptr[1],indent+3);
                break;
	    case NOT:
	    case UMINUS:
		printf("%*c%s\n",indent,' ',T->type_id);
		displayAST(T->ptr[0],indent+3);
		break;
            case RETURN:
                printf("%*c%s\n",indent,' ',"return语句：");
                displayAST(T->ptr[0],indent+3);
                break;
	    case BREAK:
	    case CONTINUE:
	    case RETURN_E:
		printf("%*c%s\n",indent,' ', T->type_id);
		break;
	    case VOID:
		printf("%*c%s\n",indent,' ', "类型：void");
		break;				
        }
    }
}


