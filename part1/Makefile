SHELL := /bin/bash
.PHONY: compile strace diff 

compile: part1.c empty.c
	gcc -o part1.x part1.c
	gcc -o empty.x empty.c
strace: compile
	strace -o log2 ./part1.x
	strace -o log ./empty.x
diff:
	sdiff log log2 > diff.txt 
clean:
	rm *.o *.gch *.x log log2 *.txt
