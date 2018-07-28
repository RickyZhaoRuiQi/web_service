all : myhttpd

myhttpd : myhttpd.c
	gcc -o myhttpd myhttpd.c -lpthread -g

clean :
	rm myhttpd
