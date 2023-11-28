TARGET = daemon.out 
SOURCES = src/daemon.c main.c
INCLUDE = include
PID = 100

all: build

build:
	touch daemon.cfg
	gcc ${SOURCES} -I./${INCLUDE} -o ${TARGET}

run:
	./${TARGET} ${PID}

run_daemon:
	./${TARGET} ${PID} -d

clean:
	rm *.out *.cfg