all: http_client http_server

http_server: server.c
	gcc server.c -o http_server

http_client: client.c
	gcc client.c -o http_client

clean:
	rm -f *.o http_client http_server