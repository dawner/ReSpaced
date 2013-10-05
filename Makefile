CC = g++
LPATH = -L/usr/pkg/lib
LDPATH = -Wl,-R/usr/pkg/lib 
CFLAGS=-g -Wall 
LIBS=-lGL -lGLU -lglut -ljpeg -lpng15 -lm
IPATH= -I/usr/pkg/include

all: Ass4

Ass4: main.o G308_ImageLoader.o G308_Geometry.o
	$(CC) -o Ass4 main.o G308_ImageLoader.o G308_Geometry.o $(LIBS) $(LPATH) $(LDPATH)
main.o:
	$(CC) -c $(CFLAGS) main.cpp $(IPATH)  
G308_ImageLoader.o:
	$(CC) -c $(CFLAGS) G308_ImageLoader.cpp $(IPATH)
G308_Geometry.o:
	$(CC) -c $(CFLAGS) G308_Geometry.cpp $(IPATH)
clean :
	rm -rf *.o
	rm Ass4