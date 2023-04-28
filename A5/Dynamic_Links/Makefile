FLAGS = -g -O3

default: ospf

ospf: ospf_dyn.cpp
	g++ -o ospf ospf_dyn.cpp $(FLAGS)
	mkdir Outputs

clean:
	rm ospf
	rm -rf Outputs
	mkdir Outputs