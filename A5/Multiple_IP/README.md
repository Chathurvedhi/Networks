# Lab 5 OSPF Routing Algorithm

## Changes to input file

* After the first line, with num_nodes num_links, We have num_nodes lines which give information of IP's of all the routers.
```
3 2
0 172.20.10.2
1 172.20.10.5
2 172.20.10.5
0 1 4 5
1 2 4 6
``` 

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
