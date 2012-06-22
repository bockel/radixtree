RTDIR = ../src
PROGS =
UTILS = rt_build rt_search rt_iter rt_map
CFLAGS = -I$(RTDIR)
CFLAGS += -Wall
CFLAGS += ${EXTRA_CFLAGS}

ifeq ($(mode),release)
	CFLAGS += -O2
else
	mode = debug
	CFLAGS += -O0 -g
endif

TEST_TARGET = run_tests
TESTS=./do_tests

all: $(UTILS)

radixtree.o :
	$(CC) -c $(CFLAGS) $(RTDIR)/radixtree.c

cplusplus:
	cc=$(CXX) $(MAKE) all

$(PROGS) $(UTILS) : radixtree.o
	$(CC) $(CFLAGS) radixtree.o -o $@ $(@).c

check: all


.PHONY: clean

clean:
	rm -f $(UTILS) $(PROGS) *.o
