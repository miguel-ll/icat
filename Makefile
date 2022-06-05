default:
	cc -D_POSIX_C_SOURCE=2 -std=c99 -Wall -g -o icat imcat.c -lm

install:
	cc -D_POSIX_C_SOURCE=2 -std=c99 -Wall -g -o /usr/local/bin/icat imcat.c -lm

uninstall:
	rm /usr/local/bin/icat
