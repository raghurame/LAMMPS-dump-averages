all:
	gcc -o dumpAverages dumpAverages.c -lm
	./dumpAverages dumpFirst.bonds
