#Makefile client
All: client server

client: client.c
	gcc -Wall -g client.c list.c wrapper.c planetDisplayList.c -I/usr/include/cairo -L/usr/include/lib/ `pkg-config --cflags gtk+-3.0` -lcairo -o client `pkg-config --libs gtk+-3.0` -lpthread -lrt -std=gnu11

server: server.c
	gcc -Wall -g server.c list.c wrapper.c planetDisplayList.c -I/usr/include/cairo -L/usr/include/lib/ `pkg-config --cflags gtk+-3.0` -lcairo -o server `pkg-config --libs gtk+-3.0` -lpthread -lrt -lm -std=gnu11


clean:
	rm -f client server *.o
