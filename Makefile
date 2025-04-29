all: a.out

a.out: identify_interface.cpp
	g++ -O3 identify_interface.cpp


clean:
	rm -f a.out
