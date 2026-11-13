all: buildir serv cl
	mkdir -p logs/

serv: server/serv.cpp log.c
	g++ -Wall -Wextra -I. -Iserver -g -o build/serv server/serv.cpp -x c log.c

cl: client/client.cpp log.c shell.c
	g++ -Wall -Wextra -I. -Iclient -g -o build/client client/client.cpp client/handlers.cpp -x c log.c -x c shell.c

buildir:
	mkdir -p build/

clean:
	rm -rf build/serv build/client logs/
