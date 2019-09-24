---DIRECTIONS FOR USE---

---COMPILE INSTRUCTIONS---
open terminal in the directory this readme is located,
which at a minimum must contain:
	-client.c
	-server.c
	-makefile
Run 'make all' to build both the client and the server.
Run 'make http_client' to build the client.
Run 'make http_server' to build the client.
Run 'make clean' to delete both the http_client and http_server
before rebuilding.

---RUNNING JUST THE CLIENT---
open terminal in the directory in which the http_client file is located and
run the following command.
Usage: ./http_client [-options] server_url port_number
	List of available options:
	-p: prints the RTT for accessing the URL on the terminal before 
	    server's response (i.e., the webpage)
Examples:
	./http_client www.google.com 80
	./http_client -p www.mit.edu 80


---RUNNING SERVER AND CLIENT---
open terminal in the directory in which the http_client and http_server files
are located and run the following commands in the following order.
1. Find hostname by running the command 'hostname' in the correct directory
2. Start up server
3. Connect client to server
Usage for server: ./http_server port_number
Example:
	./http_server 8888
Usage for client: 
	./http_client [-options] hostname(followed by /TMDG.html) port_number
	List of available options:
	-p: prints the RTT for accessing the URL on the terminal before 
	    server's response (i.e., the webpage)
Example:
	./http_client rambo.wpi.edu/TMDG.html 8888
