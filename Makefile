CXX?=g++
CPPFLAGS+=-I. -O3
LDFLAGS+=-static
PROG=prog

SRCS := $(wildcard ./*.cpp)
OBJS := $(patsubst ./%.cpp,build/%.o,$(SRCS))

build/%.o: %.cpp
	$(CXX) -c -o $@ $< $(CPPFLAGS)


build/$(PROG): build $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o $@

all: $(PROG)

.PHONY: $(PROG)
$(PROG): build/$(PROG)

.PHONY: build
build:
	mkdir -p build

clean:
	rm -rf build
