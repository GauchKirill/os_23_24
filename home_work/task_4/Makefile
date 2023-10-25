
FLAGS = -Wall -Wextra -Wpedantic -Werror
TARGET = square
SOURCES = main.c

.PHONY: square

rebuild: clean build

build: ${SOURCES}
	gcc ${FLAGS} ${SOURCES} -o ${TARGET}

run:
	./${TARGET}

clean:
	rm ${TARGET}

