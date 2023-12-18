CC = gcc
CFLAGS = -Wall -O2 -Wextra -Wpedantic
SERVER_TARGET = server.out
SLAVE_TARET = slave.out
SERVER_SOUCES = server.c
SLAVE_SOURCES = slave.c

all: build_server build_slave

build_server: ${SERVER_SOUCES}
	${CC} ${SERVER_SOUCES} ${CFLAGS} -o ${SERVER_TARGET}

build_slave: ${SLAVE_SOURCES}
	${CC} ${SLAVE_SOURCES} ${CFLAGS} -o ${SLAVE_TARET}

clear:
	rm *.out