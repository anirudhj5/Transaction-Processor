all: bank


bank: bank.o string_parser.o
	gcc -g -pthread -Wall -o bank bank.o string_parser.o 

string_parser.o: string_parser.c string_parser.h
	gcc -g -c string_parser.c

bank.o: bank.c
	gcc -g -c bank.c


clean:
	rm -f core *.o bank 