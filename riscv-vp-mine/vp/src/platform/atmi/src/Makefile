SRCS = atmi.c examples.c
TARGET = ../lib/libatmi.a

OBJS = $(SRCS:%.c=%.o)

CC = gcc
CFLAGS  = -I../include

lib: $(TARGET)

doc:
	doxygen Doxyfile

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

%.a: $(OBJS)
	@mkdir -p ${dir  $@}
	@ar cru $@ $^
	@ranlib $@

clean:
	$(RM) *.o *.d $(TARGET)

