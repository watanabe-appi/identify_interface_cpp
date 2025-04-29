all: identify_interface

identify_interface: identify_interface.cpp
	g++ -O3 identify_interface.cpp -o $@


clean:
	rm -f identify_interface
