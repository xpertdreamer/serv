all: buildir shell serv cl
	mkdir -p logs/

shell: shell.cpp
	g++ -Wall -Wextra -I. -fPIC -shared \
		shell.cpp -o build/shell.so

serv: server/serv.cpp log.c shell
	g++ -Wall -Wextra -I. -Iserver -g -o build/serv server/serv.cpp server/handler.cpp \
		-x c log.c -Lbuild -l:shell.so

cl: client/client.cpp log.c shell
	g++ -Wall -Wextra -I. -Iclient -g -o build/client client/client.cpp -x c log.c \
		-Lbuild -l:shell.so

buildir:
	mkdir -p build/

clean:
	rm -rf build/ logs/
