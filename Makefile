CXXFLAGS=-O3 -std=c++11 -pthread

.PHONY: all
all: threadpool-test

threadpool-test: threadpool-test.cc threadpool.h
	$(CXX) $(CXXFLAGS) -o $@ threadpool-test.cc

.PHONY: clean
clean:
	rm -f threadpool-test *~
