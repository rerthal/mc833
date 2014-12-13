client:
	gcc client.c net.c -Wall -o client.opt
server:
	gcc server.c net.c -Wall -o server.opt
clean:
	$(QUIET) rm -f *.opt
