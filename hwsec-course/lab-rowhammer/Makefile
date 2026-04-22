CC=g++

CXXFLAGS=-std=c++20

LIBVERIF=-Lbin -lverif -Wl,-rpath,'$$ORIGIN'

HELPER_FILES = src/shared.cc src/shared.hh src/util.hh src/params.hh

.PHONY: all clean

all: bin/libverif.so bin/part0 bin/part1 bin/part2 bin/part3 bin/part4 bin/part5

clean:
	rm -f bin/part0
	rm -f bin/part1
	rm -f bin/part2
	rm -f bin/part3
	rm -f bin/part4
	rm -f bin/part5

bin/part0: src/part0/part0.cc
	$(CC) $(CXXFLAGS) -o $@ src/part0/part0.cc

bin/part1: src/part1/part1.cc $(HELPER_FILES)
	$(CC) $(CXXFLAGS) -o $@ src/part1/part1.cc src/shared.cc $(LIBVERIF)

bin/part2: src/part2/part2.cc $(HELPER_FILES)
	$(CC) $(CXXFLAGS) -o $@ src/part2/part2.cc src/shared.cc $(LIBVERIF)

bin/part3: src/part3/part3.cc $(HELPER_FILES)
	$(CC) $(CXXFLAGS) -o $@ src/part3/part3.cc src/shared.cc $(LIBVERIF)

bin/part4: src/part4/part4.cc $(HELPER_FILES)
	$(CC) $(CXXFLAGS) -o $@ src/part4/part4.cc src/util.hh src/shared.cc $(LIBVERIF)

bin/part5: src/part5/part5.cc src/ecc.hh
	$(CC) $(CXXFLAGS) -o $@ src/part5/part5.cc $(LIBVERIF)
	
