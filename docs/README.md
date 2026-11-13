## Usage

For now, use two terminals or one of the following methods:

**1. Using the interpreter**

``` shell
$ bash run.sh
```

**2. Make the script executable (once)** 

``` shell
$ chmod +x run.sh
$ ./run.sh
```

**3. Manual way** 

``` shell
$ cd serv && {./build/serv >/dev/null 2>&1 & sleep 1.5; ./build/client localhost 8080}
```
