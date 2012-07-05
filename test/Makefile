RTDIR = ../src
UTILS = rt_build rt_get rt_prefix rt_map rt_unit_test
CFLAGS = -I$(RTDIR) -Wall -Wextra
CFLAGS += ${EXTRA_CFLAGS}

ifeq ($(mode),release)
	CFLAGS += -O2
else
	mode = debug
	CFLAGS += -O0 -g -DDEBUG
endif

all: $(UTILS)

radixtree.o : $(RTDIR)/radixtree.c
	$(CC) -c $(CFLAGS) $(RTDIR)/radixtree.c

cplusplus:
	cc=$(CXX) $(MAKE) all

$(UTILS) : radixtree.o
	$(CC) $(CFLAGS) radixtree.o -o $@ $(@).c

.PHONY: clean check

check: all
	sh run_check.sh

clean:
	rm -f $(UTILS) *.o
