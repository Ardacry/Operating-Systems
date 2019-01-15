all : syn_phistogram syn_thistogram FileCreator

syn_phistogram : syn_phistogram.c
	gcc -o syn_phistogram syn_phistogram.c -lm -lrt -pthread
syn_thistogram : syn_thistogram.c
	gcc -o syn_thistogram syn_thistogram.c -lm -lrt -pthread
FileCreator : FileCreator.c
	gcc -o FileCreator FileCreator.c -lm
clean :
	rm syn_phistogram
	rm syn_thistogram
	rm FileCreator

