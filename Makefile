


#For executing server.c
sample : echos.o echo.o


echos.o: server.c client_server.h 
	gcc -I. server.c -o echos


#For executing client.c

echo.o: client.c client_server.h
	gcc -I. client.c -o echo

#To discard previous .o files
clean:
	rm -f sample *.o core



