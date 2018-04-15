SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
DEPS = $(OBJS:.o=.d)
CFLAGS = -std=c++14
TARGET = libflyzero.a
CC = g++
NODEPS = clean

all: debug

$(TARGET): $(OBJS)
	ar rv $@ $^

.PHONY: debug
debug: CFLAGS += -g3 -O0
debug: $(TARGET)

.PHONY: release
release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

.PHONY: clean
clean:
	rm -f $(DEPS) $(OBJS) $(TARGET)

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

%.d: %.cpp
	$(CC) -MM $(CFLAGS) $< > $@

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
-include $(DEPS)
endif

