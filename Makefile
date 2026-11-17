all: buildir serv cl
	mkdir -p logs/

serv: server/serv.cpp log.c shell.cpp
	g++ -Wall -Wextra -I. -Iserver -g -o build/serv shell.cpp server/serv.cpp server/handler.cpp \
		-x c log.c

cl: client/client.cpp log.c shell.cpp
	g++ -Wall -Wextra -I. -Iclient -g -o build/client shell.cpp client/client.cpp -x c log.c

buildir:
	mkdir -p build/

clean:
	rm -rf build/serv build/client logs/
