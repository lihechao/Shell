user-sh :  main.o getCommand.o execute.o y.tab.o link.o
	 gcc -o user-sh main.o y.tab.o getCommand.o execute.o link.o

getCommand.o : getCommand.c global.h
	      gcc -c getCommand.c
execute.o : execute.c global.h
	   gcc -c execute.c
y.tab.o : y.tab.c global.h
	 gcc -c y.tab.c
link.o	: link.c global.h
	 gcc -c link.c
clean :
	rm user-sh main.o y.tab.o getCommand.o execute.o link.o
