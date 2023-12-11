CLIENT_TARGET = client.out
SERVER_TARGET = server.out
CLIENT_SOUCES = client.c
SERVER_SOUCES = server.c

CC = gcc
SERVER_CFLAGS = -Wall -Wextra -Wpedantic -lpthread
CLIENT_CFLAGS = -Wall -Wextra -Wpedantic
all: build

build: build_cli build_serv

build_serv: ${SERVER_SOUCES}
	${CC} ${SERVER_SOUCES} ${SERVER_CFLAGS} -o ${SERVER_TARGET}

build_cli: ${CLIENT_SOUCES}
	${CC} ${CLIENT_SOUCES} ${CLIENT_CFLAGS} -o ${CLIENT_TARGET}

clean:
	rm ${SERVER_TARGET} ${CLIENT_TARGET}
