# Lab 5 OSPF Routing Algorithm

## Instructions to run the code

* Makefile is provided to compile the code with given commands: 
```
make ospf : compiles the code and makes directory Outputs
make clean : removes the executable ospf and gives new empty Outputs directory
```

* If you want to compile the code manually, use the following command:
```
g++ ospf.cpp -o ospf -g -O3
mkdir Outputs
```

* Please make sure to __create the Outputs directory__ before running the code.

* Run the code as follows:
```
./ospf -i <node_id> -f <input_filename> -o <outputfilename> -h <hello_interval> -a <lsa_interval> -s <spf_interval>
```
