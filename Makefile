test:
	gcc -Wno-maybe-uninitialized -march=native -O2 -fstack-protector -Wextra -Wall -I./include/ ./src/*.c test.c -o a.out

test_all:
	gcc -march=native -O2 -fstack-protector -Werror -Wextra -Wall -I./include/ ./src/*.c test.c -o a.out
