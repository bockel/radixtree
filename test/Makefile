RTDIR = ../src
PROGS =
UTILS = rt_build rt_search rt_iter
CFLAGS = -I$(RTDIR)
CFLAGS += -g
CFLAGS += -Wall
CFLAGS += ${EXTRA_CFLAGS}

TEST_TARGET = run_tests
TESTS=./do_tests

all: $(UTILS)

radixtree.o :
	$(CC) -c $(CFLAGS) $(RTDIR)/radixtree.c

cplusplus:
	cc=$(CXX) $(MAKE) all

$(PROGS) $(UTILS) : radixtree.o
	$(CC) $(CFLAGS) radixtree.o -o $@ $(@).c

.PHONY: clean

clean:
	rm -f $(UTILS) $(PROGS) *.o
