CC = gcc
EXE = mywget
CFLAGS = -Wall

SRCS = main.c http.c url.c socket.c
OBJS = main.o http.o url.o socket.o

$(EXE): $(OBJS)
	$(CC) -o $@ $^

$(OBJS): $(SRCS)

clean:
	-rm -f $(OBJS) $(EXE)
