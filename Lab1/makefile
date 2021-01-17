miniC: lex.l parser.y ast.c def.h
	bison -d -v parser.y
	flex lex.l
	cc -o miniC lex.yy.c parser.tab.c ast.c

clean:
	rm -rf lex.yy.c
	rm -rf parser.tab.h
	rm -rf parser.tab.c
	rm -rf parser.output
	rm -rf miniC
