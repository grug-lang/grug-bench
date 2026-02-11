CC:= gcc
COMMON_FLAGS := -Wall -Wextra -Werror -ggdb
OPT_LEVEL := -O3

run: build
	./smoketest.exe

check: 
	$(CC) $(COMMON_FLAGS) bench.c -fsyntax-only 
	$(CC) $(COMMON_FLAGS) smoketest.c -fsyntax-only 

build: bench.dll smoketest.exe

bench.dll: bench.c bench.h
	$(CC) $(COMMON_FLAGS) $(OPT_LEVEL) bench.c -o bench.dll -g -shared

smoketest.exe: smoketest.c bench.h
	$(CC) $(COMMON_FLAGS) $(OPT_LEVEL) smoketest.c -o smoketest.exe -g 

clean: 
	del smoketest.exe
	del bench.dll
