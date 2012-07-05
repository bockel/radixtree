RTDIR = ../src
UTILS = rt_build rt_get rt_prefix rt_map
UNIT_TEST = rt_unit_test
CFLAGS = -I$(RTDIR) -Wall -Wextra
CFLAGS += ${EXTRA_CFLAGS}

ifeq ($(mode),release)
	CFLAGS += -O2 -D_FORTIFY_SOURCE=2 -fexceptions -DNDEBUG
else
	mode = debug
	CFLAGS += -O0 -g
endif

all: $(UTILS) $(UNIT_TEST)

radixtree.o : $(RTDIR)/radixtree.c
	$(CC) -c $(CFLAGS) $(RTDIR)/radixtree.c

cplusplus:
	cc=$(CXX) $(MAKE) all

$(UTILS) : radixtree.o
	$(CC) $(CFLAGS) radixtree.o -o $@ $(@).c

$(UNIT_TEST) : radixtree.o
	$(CC) $(CFLAGS) -w radixtree.o -o $@ $(@).c

.PHONY: clean check

check: all
	sh run_check.sh

clean:
	rm -f $(UTILS) *.o
