all: main waitforme

main: main.c
	gcc -g -std=c11 -Wall -o main main.c

waitforme: waitforme.c
	gcc -g -std=c11 -Wall -o waitforme waitforme.c

clean:
	rm -f main
	rm -f waitforme
	rm -rf *.dSYM