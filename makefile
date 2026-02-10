CC:= gcc
COMMON_FLAGS := -Wall -Wextra -Werror

run: build
	./smoketest.exe

check: 
	$(CC) $(COMMON_FLAGS) main.c -fsyntax-only 
	$(CC) $(COMMON_FLAGS) smoketest.c -fsyntax-only 

build: main.dll smoketest.exe

main.dll: main.c main.h
	$(CC) $(COMMON_FLAGS) main.c -o main.dll -g -shared -Wl,--subsystem,windows

smoketest.exe: smoketest.c
	$(CC) $(COMMON_FLAGS) smoketest.c -o smoketest.exe -g 

clean: 
	del smoketest.exe
	del main.dll
