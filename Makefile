CFLAGS = -g -std=c++14
TARGET = libFlyZero.a
OBJ = Hex.o Chunked.o Memory.o fdset.o Thread.o ThreadPool.o
CC = g++

${TARGET}: ${OBJ}
	ar rv $@ $^

Hex.o: Hex.cpp
	${CC} $^ ${CFLAGS} -o $@

Chunked.o: Chunked.cpp
	${CC} $^ ${CFLAGS} -o $@

Memory.o: Memory.cpp
	${CC} $^ ${CFLAGS} -o $@

fdset.o: fdset.cpp
	${CC} $^ ${CFLAGS} -o $@

Thread.o: Thread.cpp
	${CC} $^ ${CFLAGS} -o $@

ThreadPool.o: ThreadPool.cpp
	${CC} $^ ${CFLAGS} -o $@
	

.PHONY: clean

clean:
	@rm ${OBJ} ${TARGET} 
