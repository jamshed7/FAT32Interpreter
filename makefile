CPP = g++
all: mfs

mfs: mfs.c
	$(CPP) mfs.c -o mfs && mkdir build/ && cp mfs build/ 

test_run:
	./mfs

prod_run:
	./build/mfs

clean:
	rm -rf build && rm -f *.txt && rm -f mfs && rm -f *.out && rm -f *.o