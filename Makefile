all: client gestor

gestor: gestor.o scliente.h sconexion.h seguimiento.h sgestor.h smensaje.h stweet.h
	gcc -g -o gestor gestor.o

gestor.o: gestor.c scliente.h sconexion.h seguimiento.h sgestor.h smensaje.h stweet.h
	gcc -g -c gestor.c

client: client.o scliente.h sconexion.h seguimiento.h sgestor.h smensaje.h stweet.h
	gcc -g -o client client.o

client.o: client.c scliente.h sconexion.h seguimiento.h sgestor.h smensaje.h stweet.h
	gcc -g -c client.c 
