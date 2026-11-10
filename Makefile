all: serv client
	mkdir -p logs/

serv: serv.cpp log.c
	g++ -Wall -Wextra -o serv serv.cpp -x c log.c

client: client.cpp log.c shell.c
	g++ -Wall -Wextra -o client client.cpp handlers.cpp -x c log.c -x c shell.c

clean:
	rm -rf serv client logs/
