# Compiler Principle Experiment
HUST, CSE, IS, 2020 Compiler Principle Experiment
## 开发环境
1. 开发环境: Ubuntu 18.04 (Linux)
2. 开发语言: C
## 项目生成
Lab1~Lab4对应4次实验
在Linux环境下使用`make`命令即可编译生成可执行文件`miniC`
## 文件说明
`miniC`为可执行程序(Linux环境)  
`test.c--`文件为测试用源代码, `test.out`为对应输出重定向文件  
`parser.y`为bison语法分析文件  
`lex.l`为flex词法分析文件  
`def.h`为整体的数据结构定义的头文件  
`semantic.h`为语义分析相关函数声明的头文件  
`ast.c`为抽象语法树的构建与输出的代码  
`semanticAnalysis.c`为语义分析相关代码(包括符号表和中间代码生成)  
`semanticCases.c`为语义分析中各种文法符号的case的函数代码  
`expCases.c`为语义分析中表达式相关的case的函数代码  
`objCode.c`为目标代码生成的函数代码
## MiniC语法规则说明
### Lab1
1. 增加了char类型变量和数组类型
2. 增加了break, continue和for循环语句
3. 增加了+=, -=, *=, /= 复合运算符
4. 增加了无返回值的return语句以及函数void类型
### Lab2~Lab3
1. 考虑到后续实现难度, 对文法进行了删减: 删除了数组类型和for循环语句.
2. 不支持外部变量初始化
3. 不支持char类型变量算术计算
4. 实现了取负, 逻辑与或非等运算,
5. 实现了break, continue跳转语句
### Lab4
1. 采用最简单的朴素寄存器分配方法
2. 只支持整数类型运算
3. 函数体中必须含有`return`语句进行返回.
* PS: 后续实验基于先前实验完成, 存在后续实验对先前实验代码进行调整后未及时修改原实验代码. 以最终版本Lab4中代码为准.
   程序中仍可能存在未发现的错误.
