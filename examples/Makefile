map: pmparser.o map.o
	gcc map.o pmparser.o -o map

map.o: map.c
	gcc -c map.c -o map.o
pmparser.o:../pmparser.c
	gcc -c ../pmparser.c -o pmparser.o
