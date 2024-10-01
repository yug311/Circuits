all: third

third: third.c
	gcc -Wall -Werror -fsanitize=address,undefined -g third.c -o third

clean:
	rm -rf third *~