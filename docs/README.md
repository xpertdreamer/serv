## Usage

For now you have to use two terminals

``` shell
$ bash run.sh
```

or

``` shell
$ chmod +x run.sh
$ ./run.sh
```

or manually

``` shell
$ cd serv && {./build/serv >/dev/null 2>&1 & sleep 1.5; ./build/client localhost 8080}
```
