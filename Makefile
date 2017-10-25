SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))
CFLAGS = -c -std=c++14
TARGET = libflyzero.a
CC = g++

all: release

${TARGET}: ${OBJ}
	ar rv $@ $^

%.o: %.cpp
	${CC} ${CFLAGS} $^ -o $@

.PHONY: debug
debug: CFLAGS += -g3
debug: ${TARGET}

.PHONY: release
release: CFLAGS += -O2 -DNDEBUG
release: ${TARGET}

.PHONY: clean

clean:
	@rm ${OBJ} ${TARGET} 
