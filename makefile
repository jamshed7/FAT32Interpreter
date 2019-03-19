CPP = g++
CPPFlags = -Wall
all: mfs

mfs: mfs.cpp
	$(CPP) $(CPPlags) mfs.cpp -o mfs

run:
	./mfs

clean:
	rm -f *.o mfs *.out