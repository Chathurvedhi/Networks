FLAGS = -g -O3

default: ospf

ospf: ospf.cpp
	g++ -o ospf ospf.cpp $(FLAGS)
	mkdir Outputs

clean:
	rm ospf
	rm -rf Outputs
	mkdir Outputs