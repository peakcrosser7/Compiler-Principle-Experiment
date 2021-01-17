miniC: lex.l parser.y ast.c def.h
	bison -d -v parser.y
	flex lex.l
	gcc -o miniC ast.c parser.tab.c lex.yy.c semanticAnalysis.c semanticCases.c expCases.c

clean:
	rm -rf lex.yy.c
	rm -rf parser.tab.h
	rm -rf parser.tab.c
	rm -rf parser.output
	rm -rf miniC
