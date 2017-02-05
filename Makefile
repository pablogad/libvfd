SRCS:=pt6312.c timing.c

OBJS:=$(patsubst %.c,%.o,$(SRCS))

CFLAGS:=$(CFLAGS) -g -I.

test: libvfd.a test.o
	@echo "Link test program"
	@gcc -g -o test test.o libvfd.a -lbcm2835

libvfd.a: $(OBJS)
	@echo "Creating libvfd.a"
	@ar rvs $@ $(OBJS)

