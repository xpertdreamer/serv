all: a b

b:
	gcc -o b b.c

a:
	gcc -o a a.c

clean:
	rm -f a b
