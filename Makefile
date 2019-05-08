HEADERS = erlang.h
OBJ3 = 3.o linked_list.o poisson.o
OBJ2 = 2.o linked_list.o
SOBJ = erlang.o
OBJ = 1a.o 1b.o $(OBJ2) $(SOBJ)
LIBS = -lm
CFLAGS =


CC = clang

default: main

rand.o: rand.cc
	$(CXX) -c rand.cc -o rand.o

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -g -c $< -o $@

1a: 1a.o $(SOBJ)
	$(CC) 1a.o $(SOBJ) -o $@ $(LIBS)

1b: 1b.o $(SOBJ)
	$(CC) 1b.o $(SOBJ) -o $@ $(LIBS)

2: $(OBJ2) $(SOBJ)
	$(CC) $(CFLAGS) $(OBJ2) $(SOBJ) -o $@ $(LIBS)

3: $(OBJ3) $(SOBJ)
	$(CC) $(CFLAGS) $(OBJ3) $(SOBJ) -o $@ $(LIBS)

main: main.o linked_list.o erlang.o poisson.o
	$(CC) $(CFLAGS) main.o erlang.o poisson.o -g -o $@ $(LIBS)

clean:
	-rm -f $(OBJ)
	-rm -f 1a 1b 2 *.o
