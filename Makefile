SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))
CFLAGS = -g -c -std=c++14
TARGET = libFlyZero.a
CC = g++

${TARGET}: ${OBJ}
	ar rv $@ $^

%.o: %.cpp
	${CC} ${CFLAGS} $^ -o $@

.PHONY: clean

clean:
	@rm ${OBJ} ${TARGET} 
