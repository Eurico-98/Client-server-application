CC	= gcc
OBJS	= Servidor.o
PROG	= servidor
OBJS2	= Clients.o
PROG2	= cliente
OBJS3	=proxy.o
PROG3	=proxy

# GENERIC

all:		${PROG} ${PROG2} ${PROG3}

clean:
		rm ${OBJS} ${PROG}
		rm ${OBJS2} ${PROG2}
		rm ${OBJS3} ${PROG3}

${PROG}:	${OBJS}
		${CC} ${OBJS} -g -lm -lpthread -o $@

${PROG2}:	${OBJS2}
		${CC} ${OBJS2} -g -lm -lpthread -o $@

${PROG3}:	${OBJS3}
		${CC} ${OBJS3} -g -lm -lpthread -o $@
.c.o:
		${CC} $< -Wall -c -o $@

###########################

Servidor.o: Servidor.c

Clients.o: Clients.c

proxy.o: proxy.c
###########################
