all: a b

serv:
	g++ -Wall -Wextra -o serv serv.cpp

b:
	gcc -o b b.c

a:
	gcc -o a a.c

clean:
	rm -f a b
