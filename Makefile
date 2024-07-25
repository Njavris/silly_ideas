CXX?=g++
CPPFLAGS+=-I. -O3
LDFLAGS+=-static
PROG=prog
BUILDDIR=build

SRCS := $(wildcard ./*.cpp)
OBJS := $(patsubst ./%.cpp,$(BUILDDIR)/%.o,$(SRCS))

$(BUILDDIR)/%.o: %.cpp
	$(CXX) -c -o $@ $< $(CPPFLAGS)

$(BUILDDIR)/$(PROG): $(BUILDDIR) $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o $@

all: $(PROG)

$(PROG): $(BUILDDIR)/$(PROG)

.PHONY: $(BUILDDIR)
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)

run: $(PROG)
	./$(BUILDDIR)/$(PROG)
