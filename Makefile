all: a b

serv: serv.cpp log.c
	g++ -Wall -Wextra -o serv serv.cpp -x c log.c


clean:
	rm -f a b serv
