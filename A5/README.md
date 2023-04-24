# Lab 5 OSPF Routing Algorithm

## Instructions to run the code

* Makefile is provided to compile the code. Run `make` to compile the code. If not just the following command to compile the code.
```cpp
g++ ospf.cpp -o ospf -g -O3
```
* Run the code as follows:
```cpp
./ospf -i <node_id> -f <input_filename> -h <hello_interval> -a <lsa_interval> -s <spf_interval>
```
