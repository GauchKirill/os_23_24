
FLAGS = -Wall -Wextra -Wpedantic -lpthread
TARGET = square
SOURCES = main.c

.PHONY: square
all: build run clean

rebuild: clean build

build: ${SOURCES}
	gcc ${FLAGS} ${SOURCES} -o ${TARGET}

run:
	./${TARGET}

clean:
	rm ${TARGET}

