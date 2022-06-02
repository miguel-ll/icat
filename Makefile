default:
	cc -D_POSIX_C_SOURCE=2 -std=c99 -Wall -g -o imcat imcat.c -lm

install:
	cc -D_POSIX_C_SOURCE=2 -std=c99 -Wall -g -o /usr/local/bin/imcat imcat.c -lm

uninstall:
	rm /usr/local/bin/imcat
