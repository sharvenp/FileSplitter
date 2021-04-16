
default: clean
	gcc -Wall -g -o split split.c
	gcc -Wall -g -o assemble assemble.c

clean:
	-rm split
	-rm assemble