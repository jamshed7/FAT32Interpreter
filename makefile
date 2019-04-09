CPP = g++
all: mfs

mfs: mfs.c
	$(CPP) mfs.c -o mfs && mkdir build/ && cp mfs build/ && rm -f mfs

run:
	./build/mfs

clean:
	rm -rf build && rm -f *.txt