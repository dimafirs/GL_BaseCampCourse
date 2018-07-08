TARGET=determ_calc
DEPS=functions
DEPS:=$(addsuffix .o, $(DEPS))

CC=gcc
GFLAGS=-O3

all: $(TARGET) clean

$(TARGET): $(DEPS)
	$(CC) $(CFLAGS) $(addsuffix .c, $(TARGET)) $(DEPS) -o $@ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f *.o
