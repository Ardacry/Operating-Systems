all : phistogram thistogram FileCreator

phistogram : phistogram.c
	gcc -o phistogram phistogram.c -lm
thistogram : thistogram.c
	gcc -pthread -o thistogram thistogram.c -lm
FileCreator : FileCreator.c
	gcc -o FileCreator FileCreator.c -lm
clean :
	rm phistogram
	rm thistogram
	rm FileCreator
