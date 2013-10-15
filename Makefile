# Stewart's home system
# CC = g++
# LPATH = -L/usr/pkg/lib -L/usr/local/libpng/lib 
# LDPATH = -Wl,-R/usr/pkg/lib 
# CFLAGS=-g -Wall 
# LIBS=-lGL -lGLU -lglut -ljpeg -lpng15 -lm
# IPATH= -I/usr/pkg/include

# Generic linux/uni
# CC = g++
# LPATH = -L/usr/pkg/lib
# LDPATH = -Wl,-R/usr/pkg/lib
# CFLAGS=-g -Wall
# LIBS=-lGL -lGLU -lglut -ljpeg -lpng15 -lm
# IPATH= -I/usr/pkg/include

# Windows (Dawn)
CC = g++
LPATH = -L/usr/pkg/lib
LDPATH = -Wl,-R/usr/pkg/lib
CFLAGS=-g -Wall
LIBS=-lopengl32 -lglu32 -lglut32 -lm
IPATH= -I/usr/pkg/include

all: Retraced

Retraced: main.o CollisionSystem.o G308_Geometry.o ParticleSystem.o SculptObject.o
	$(CC) -o Retraced main.o CollisionSystem.o G308_Geometry.o ParticleSystem.o SculptObject.o $(LIBS) $(LPATH) $(LDPATH)
main.o:
	$(CC) -c $(CFLAGS) main.cpp $(IPATH)  
CollisionSystem.o:
	$(CC) -c $(CFLAGS) CollisionSystem.cpp $(IPATH)
G308_Geometry.o:
	$(CC) -c $(CFLAGS) G308_Geometry.cpp $(IPATH)
ParticleSystem.o:
	$(CC) -c $(CFLAGS) ParticleSystem.cpp $(IPATH)
SculptObject.o:
	$(CC) -c $(CFLAGS) SculptObject.cpp $(IPATH)
clean :
	rm -rf *.o
	rm -f Retraced
	rm -f Retraced.exe