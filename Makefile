CPP=g++
CPPFLAGS=-I.
PROG=prog

SRCS := $(wildcard ./*.cpp)
OBJS := $(patsubst ./%.cpp,bin/%.o,$(SRCS))

bin/%.o: %.cpp
	$(CPP) -c -o $@ $< $(CPPFLAGS)


bin/$(PROG): bin $(OBJS)
	$(CPP) $(LDFLAGS) $(OBJS) -o $@

all: $(PROG)

.PHONY: $(PROG)
$(PROG): bin/$(PROG)

.PHONY: bin
bin:
	mkdir -p bin

clean:
	rm -rf bin
