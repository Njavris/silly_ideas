CPP=g++
CPPFLAGS=-I.
PROG=prog

SRCS := $(wildcard ./*.cpp)
OBJS := $(patsubst ./%.cpp,bin/%.o,$(SRCS))

bin:
	mkdir bin

bin/%.o: %.cpp bin
	$(CPP) -c -o $@ $< $(CPPFLAGS)

$(PROG): $(OBJS)
	$(CPP) $(LDFLAGS) $(OBJS) -o bin/$(PROG)

all: $(PROG)

clean:
	rm -rf bin
